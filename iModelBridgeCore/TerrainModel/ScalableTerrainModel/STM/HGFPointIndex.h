//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/STM/HGFPointIndex.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once

#include <ImagePP/all/h/HGF3DExtent.h>
#include <STMInternal/Storage/IDTMTypes.h>
#include <STMInternal/Storage/IDTMFile.h>

#include <ImagePP/all/h/HPMPooledVector.h>

#include <ImagePP/all/h/HGF3DCoord.h>
#include <ImagePP/all/h/HGF2DTemplateExtent.h>
#include <ImagePP/all/h/HVE2DSegment.h>
#include "HGFSpatialIndex.h"
#include "HGFPointTileStore.h"
#include <ImagePP/all/h/HVEShape.h>

// Predeclaration of the Point Index Filter interface. This interface is defined lower in this same file.
template<class POINT, class EXTENT> class IHGFPointIndexFilter;

/** -----------------------------------------------------------------------------

    The following classes implement a point index. The index is based on a quadtree
    (or octtree) structure. The architecture of the index makes it so that a single
    function needs to be overloaded in order to change the spliting algorithm that
    ultimately dictates if the spatial index has 4 or 8 sub-nodes and what is their
    location into space.

    In order to be the most general, the point container type is undecided, yet this
    container must comply with a simplified STL std::vector interface a few of these
    function can be overriden if needed.

    A few flavors of the index are already provided, the most notable of which is the
    stored point index. The type of the store is implementation specific but must comply
    with the IDataStore interface for the DataType specific to the point.

    The point index struture derives from a prototype specifically designed for managing
    DTM (Digital Terrain Model) points which usually contain 2.5D set of points.
    The point index structure recognises the necessity of providing various options
    for various needs. The base structure of the index is a quadtree and it can be
    configured and used as a plain quadtree, where the points indexed will always be
    stored at a leaf of the quadtree index. The point index however does provide a
    filtering mechansim that allows to position points at various non-leaf levels
    in the quadtree to increase or diminish the importance or accessibility of some points.

    FILTERING
    This mechanism is generic in the sense the actual filtering algorithm is completely independent
    from the indexing within the limitations that the filtering mechanism will operate
    upon either a node and its subnodes OR on a leaf node. The filtering mechanism in these
    two cases can be completely different. When the filtering process is executed, then
    the filtering object is called iteratively starting at the leaves and moving toward
    the root. The process guarantees that a subnodes has already been filtered prior
    to calling the filtering process for a parent node. The filtering process
    is simply given a nodes and a list of subnodes to this node. The number of sub-nodes
    can vary; typically it will be 1, 4 or 8 depending on the index configuration selected.
    The filtering process is free to remove points from any of the nodes provided (either parent or subnodes).

    A typical use of filtering would be to remove duplicate points, or to duplicate points
    we want to move to parent nodes based on whatever criterium.

    The filter must annouce right away wether it will or will not duplicate points during the filtering
    process. If points are not duplicated, and points are simply given more spatial importance
    by moving them to parent nodes, then the filtered is said to be PROGRESSIVE. If points are
    duplicated then the filter is not progressive. This progressiveness will be important during
    spatial query process.


    INDEX STRUCTURE
    The index structure is based mainly on the quadtree architecture, however some configurations
    change this structure in some case. When the index is populated with points, the
    points fill index nodes. Initially there is a single node which gets filled and
    the extent of the node content increases adequately to include points added. The extent
    is maintained square to prevent the construction of long and thin node extents that could result from
    ordered points sets provided as strip (such as resulting from reading a stripped DEM raster elevation
    file). When the number of points in the node reaches some split treshold amount of points,
    the node is split and the points of the split node are distributed to the created sub-nodes, initially
    completely emptying the split node (subsequent filtering may repopulate back the new parent node eventually).
    Once the alignement is set and the root node has split, it is still possible to add points outside
    the root node extent, but this will simply trigger the creation of a new root node and the initial root node
    will become one of the subnodes. The new root extent alignement is based upon the initial root extent
    and the location depend on the point being added.

    Once the first root node is split, the extent of all nodes cannot be modified arbitrarily,
    and the node alignement becomes fixed. The split function can be overloaded and configure the
    following behaviors. The SplitNode() method must create any number of sub-nodes but the
    union of the extent of the subnodes must cover exactly the same area as the split nodes. Usually
    the sub-node extents will be disjoint one from another, yet no ill effect were identified about a possible
    overlapping sub-node extents. The default implementation will split node in equal size sub-nodes
    thus splitting the original extent in two in both dimensions (or three dimensions if octal split is used).

    The index recognises the concept of LEVEL. The level is the depth within the index from the root. Thus the root level is always
    0, its sub-nodes are at level 1, and so on for the sub-nodes of the sub-nodes. It is possible to impose the
    balancing of the index. If the index is balanced, then the index will guarantee that the depth of the leaf nodes
    will be the same for the whole tree, regardless of the density of points and the split treshold. This means that
    if a leaf is being split as a result of containing more points than allowed by the split treshold
    then it will first split, then propagate a message through all the index nodes about the final
    depth required. All leaf nodes that do not satisfy this depth level will increase their depth typically through splitting.

    The concept of index balancing result from the necessity to homogenize point density or point filtering
    density across a same level. This does not necessarily mean that the amount of points between nodes of a same
    level are similar, on the contrary. It means that the point density of a node can be related by a predictable
    rule to the density of the leaf nodes after filtering. For example, assuming the filtering process will takes all
    points of a sub-node and populate the parent node with 1 node out of every four points in the subnodes. If the
    subnodes have high density compared to other sub-nodes of the same level then the parent node will also be high density relatively
    to the nodes at this same parent node level.

    Splitting nodes in 4 sub-nodes just to increase the level depth, is not an economical
    structure in matter of points per node, for this reason the index will increase level by simply
    creating a single and unique subnode. In this case the index remains balanced yet deviates significantly from the
    normal quadtree structure. The result greatly minimizes the amount of nodes in the index yet provides the same
    spatial indexing and balancing capabilities. It is the optimal structure if index balancing is required.
    Given this fact, the filtering process must be implemented in such a way as to filter nodes that have 4 sub-nodes or a single sub-node.
    The number of sub-nodes will be provided to the filtering process, and the filter can assume that if the number is 1 then
    the sub-node was created for level balancing purposes. Note that as the index is populated, it may happen
    that a leaf node that happens to be the single sub-node of a node will have to split as the amount of
    points has reached the split treshold. In this case, the node will NOT be split, It will instead trigger and
    event to its parent node, for this parent node to be split instead. If this occurs then the single-sub-node will be replaced
    by 4 sub-nodes. Since this process will occur during point addition, the process may be temporarily delayed
    after the addition process has completed, resulting in a node having momentarily more points than normally allowed.
    Notice that if the single node subdivision is many levels deep, the split process will always occur at the most
    rootward node that has not been split.

    MEMORY MANAGEMENT

    The index allows for optional memory management mainly but not exclusively through the point container.
    Typical use, will require the indexing a thousand, millions or even billions of points. Not all
    points can then be maintained in memory. An optional point container called
    HPMStoredPooledVector provides pooled memory management. The principle is that this
    vector memory location is given to the management of a pool. Usually all node points
    container will share the same pool (or node points container from many point indexes).
    Whenever points are needed, the pool will be asked for memory. If the maximum number of
    points is attained, before allocating required memory, it will first ask the least recently
    used item (the point container) to discard its memory. The details of the discard process are
    the only known to the pool item itself, but whatever the discard process implementation, the memory used will become available back.
    The pool manager will trigger the discard of as many pool item is required to obtain enough
    free memory to allocate the required amount.

    In the case of the provided HPMStoredPooledVector, the discard process is implemented
    such that if the data has not been modified since last loaded from provided store, it simply is deallocated.
    otherwise the storage of the data is given to the store object. The actual implementation of the store
    is irrelevant. The vector is garanteed that the data will be stored and can be loaded back when required.

    Any pool item, prior to accessing its managed data must make sure it is available. If the memory is currently in
    a discarded state, the data must first be inflated. The inflate process will recuperate the data after the
    pool manager has provided the required memory. In the specific case of the HPMStoredPooledVector, the
    points are loaded back from the store.

    PERMANENT STORAGE AND DATA RETRIEVAL

    In the above memory management description, everything is kept under the control of the
    point container alone and the index must not bother about the fact memory management occurs. However,
    the storage of the points alone is not sufficient to insure complete information storage. The index and the
    index nodes contain information that must also be stored along the points in the case permanent storage is needed.
    Such information include
    the node extent, the sub-node identification, and any additional control data required. For this reason a stored index implementation
    is provided. This stored index requires and knowns some details about the store that may be used by the
    point container. In this case, whenever data is stored, the stored index nodes will also be called and be given
    the chance to store additional data related to the node. Likewise the index master information will
    also be able to store its information as well at the end of the process or as a result of an explicit storage request.

    A stored index structure will make exclusive use of stored index nodes which are descendants to the ancester
    point node index.

    When permanent storage of the index is involved, the index may be initially fully stored and no information
    initially loaded. When the index is created by providing a store, the index will immediately try to
    load the master control information and the root node control information. No other information will be loaded.
    When a node is accessed then the node control information (extent, number of sub-nodes ...) is loaded
    from the store. The points are not initally loaded unless required. The node and index control information
    will never be completely discarded but will remain in memory for the life of the index. Once points are accessed, then
    point load will occur.

    The stored index will be able to make use of any IHGFPointIndexStore compliant object regardless of actual
    implementation. It will however recognise the fact some storage can be slow to operate. For example, the
    data could be stored on a remote file, or in a remote database. In such case data retrieval
    can be a lenghty process. The index will allow, in very specific case, not to wait for this data retieval
    process to conclude before returning control back to the caller. This will occur mainly when point
    query is performed. If the data is not readily available, then the query request will be delayed till the
    data arrives from the store. Meanwhile the caller will continue operation. When the data finally arrives,
    then the query process will complete and the initial requester of the data will be advised the query result is completed.

    INDEX MODIFICATION

    The index implementation was initally based on the assumption that no data modification will occur
    except for the addition of new points. Of course this assumption can only be considered temporary as the purpose of have data
    to manage is to manage this data. Point addition and removal, as well as modification will occur. For this
    reason the index provides a full interface for such operation. As modification of data (including point addition) may
    have a significant impact on the data after the filtering process has been performed, some possible
    configurations were also provided for this purpose. The default behavior will be that the filtering process
    will not occur unless the filtering is required. If a node, or one of its sub-nodes (or sub-node to subnode ...) has been
    spatially modified, then the node will be tagged as Needing Filtering. This will not result in filtering process
    being performed anew. Instead, the filtering process will be delayed till the filtering is required.

    The factors that will trigger filtering are various. The first is that the node points is
    accessed and that it needs filtering. Notice that needing filtering results from the node content having been
    modified or one of its sub-nodes needing filtering (recursively). The next depends on the
    filter. Some filtering scheme will tolerate nicely that a node be moved if it stays
    inside the same node. If the filtering considxers no spatial relation with the weight or importance
    of the point, then moving it within the node has little importance. Another filtering process
    may be fussy enough that modifying less that 10% of the points has little impact. The same
    can hold for point removal.

    -----------------------------------------------------------------------------
*/
template<class POINT, class EXTENT> class IHGFPointIndexFilter;
template<class POINT, class EXTENT> class IHGFPointIndexQuery;
//template <class POINT, class EXTENT> class HGFPointTileStore;
template<class POINT, class EXTENT> class HGFPointIndex;

//MST : Should be redesigned when some progress/termination mechanism is 
//      implemented.
typedef int (*CheckStopCallbackFP)(long dtmFeatureType);

extern CheckStopCallbackFP g_checkIndexingStopCallbackFP;



/** -----------------------------------------------------------------------------

    This class implements a point index node.

    -----------------------------------------------------------------------------
*/


template <class POINT, class EXTENT> class HGFPointIndexNode : public HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<HGFPointIndexNode<POINT, EXTENT> >, HGFPointNodeHeader<EXTENT> >
    {
    friend class IHGFPointIndexFilter<POINT, EXTENT>;
    friend class HGFPointIndex<POINT, EXTENT>;

    // That is not an error ... we make the ancester class a friend to self class so that the ancester can call protected overriden virtual methods
    // of the present class ... certainly very weird but valid nevertheless
    friend class HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<HGFPointIndexNode<POINT, EXTENT> >, HGFPointNodeHeader<EXTENT> >;

public:


     

    /**----------------------------------------------------------------------------
     Constructor for node based upon extent only.

     @param pi_SplitTreshold - The split treshold value used to determine when a
                               leaf node must be split. It is the maximum number of
                               points referenced by node before a split occurs.

     @param pi_rExtent - The extent of the node

     @param pool IN The memory manager pool to be used. This pool is used to limit the
     number of points in memory at any given time.

     @param store IN The store to which points are stored. This store is used in conjunction with
        pool management system.

     @param filter IN This filter is used to filter points through the index tree.
     The purpose of the filter is to promote most valuable points to upper levels of
     the index so they are used more often

     @balanced IN If true the index will be balanced. The index has then the same depth through
     all the branches.
    -----------------------------------------------------------------------------*/
    HGFPointIndexNode(size_t pi_SplitTreshold,
                      const EXTENT& pi_rExtent,
                      HFCPtr<HPMCountLimitedPool<POINT>> pool,
                      HFCPtr<HGFPointTileStore<POINT, EXTENT> > store,
                      IHGFPointIndexFilter<POINT, EXTENT>* filter,
                      bool balanced,
                      bool propagateDataDown);

    /**----------------------------------------------------------------------------
     Constructor for node based upon parent node and extent.

     @param pi_SplitTreshold - The split treshold value used to determine when a
                              leaf node must be split. It is the maximum number of
                              objects referenced by node before a split occurs.

     @param pi_rExtent - The extent of the node

     @param pi_rpParentNode - Reference to pointer to parent node for created node
     store, pool and filter information is obtained from this parent node.
    -----------------------------------------------------------------------------*/
    HGFPointIndexNode<POINT, EXTENT> (size_t pi_SplitTreshold,
                                      const EXTENT& pi_rExtent,
                                      const HFCPtr<HGFPointIndexNode<POINT, EXTENT> >& pi_rpParentNode);

    /**----------------------------------------------------------------------------
     Constructor for node based upon parent node and extent.

     @param pi_SplitTreshold - The split treshold value used to determine when a
                              leaf node must be split. It is the maximum number of
                              objects referenced by node before a split occurs.

     @param pi_rExtent - The extent of the node

     @param pi_rpParentNode - Reference to pointer to parent node for created node
     store, pool and filter information is obtained from this parent node.

     @param IsUnsplitSubLevel - This parameter indicates to the created node wether
     it is an unsplit sub node. In this case, it knowns that its parent is not
     split and upon requiring a split, the parent should be requested to do it.
    -----------------------------------------------------------------------------*/
    HGFPointIndexNode<POINT, EXTENT> (size_t pi_SplitTreshold,
                                      const EXTENT& pi_rExtent,
                                      const HFCPtr<HGFPointIndexNode<POINT, EXTENT> >& pi_rpParentNode,
                                      bool IsUnsplitSubLevel);

    /**----------------------------------------------------------------------------
     Copy constructor - This constructor may only be called when the
     given node has no parent

     @param pi_rNode - Reference to node to duplicate. This node must have no parent
    -----------------------------------------------------------------------------*/
    HGFPointIndexNode<POINT, EXTENT> (const HGFPointIndexNode<POINT, EXTENT>& pi_rNode);

    /**----------------------------------------------------------------------------
     Alternate Copy constructor ... equivalent but parent node is provided as parameter

     @param pi_rNode - Reference to node to duplicate.

     @param pi_rpParentNode - Pointer to parent node
    -----------------------------------------------------------------------------*/
    HGFPointIndexNode<POINT, EXTENT>(const HGFPointIndexNode& pi_rNode,
                                     const HFCPtr<HGFPointIndexNode>& pi_rpParentNode);

    /**----------------------------------------------------------------------------
     Constructor for node loaded from file. Only the header is loaded from the file
     The actual points are kept on disk

     @param pi_rpParentNode - Reference to pointer to parent node for created node

     @param pool IN The memory manager pool to be used. This pool is used to limit the
     number of points in memory at any given time.

     @param store IN The store to which points are stored. This store is used in conjunction with
        pool management system.

     @param filter IN This filter is used to filter points through the index tree.
     The purpose of the filter is to promote most valuable points to upper levels of
     the index so they are used more often


    -----------------------------------------------------------------------------*/
    HGFPointIndexNode(HPMBlockID blockID,
                      HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parent,
                      HFCPtr<HPMCountLimitedPool<POINT> > pool,
                      HFCPtr<HGFPointTileStore<POINT, EXTENT> > store,
                      IHGFPointIndexFilter<POINT, EXTENT>* filter,
                      bool balanced,
                      bool propagateDataDown);

    /**----------------------------------------------------------------------------
     Destroyer
    -----------------------------------------------------------------------------*/
    virtual ~HGFPointIndexNode<POINT, EXTENT> ();

    /**----------------------------------------------------------------------------
     Clone
     These methods create a duplicate of the self node of the same type. The node
     has no parent and no children except for the Clonechild() which automatically
     sets the parent relationship. The CloneUnsplitChild creates a child node
     that is an unsplit additional level in the index
    -----------------------------------------------------------------------------*/
    virtual HFCPtr<HGFPointIndexNode<POINT, EXTENT> > Clone () const;
    virtual HFCPtr<HGFPointIndexNode<POINT, EXTENT> > Clone (const EXTENT& newNodeExtent) const;
    virtual HFCPtr<HGFPointIndexNode<POINT, EXTENT> > CloneChild (const EXTENT& newNodeExtent) const;
    virtual HFCPtr<HGFPointIndexNode<POINT, EXTENT> > CloneUnsplitChild (const EXTENT& newNodeExtent) const;



    /**----------------------------------------------------------------------------
     Indicates that the node has been modified. For storable nodes this
     implies that the node should be stored.
     In addition indicates filtering must be reperformed.

     This method simply implements a way to discard cached pre-computed parameters
     that may have become invalid.

     This method is originally part of spatial index node interface but is
     also part of the CONTAINER class used by the point index. The present implementation
     will call both the spatial index node implementation and the ancester container
     interface.
    -----------------------------------------------------------------------------*/
    virtual void SetDirty(bool dirty) const; // Intentionaly const ... only mutable members are modified


    /**----------------------------------------------------------------------------
     Changes the store
    -----------------------------------------------------------------------------*/
    virtual bool ChangeStore(HFCPtr<HGFPointTileStore<POINT, EXTENT> > newStore);

    /**----------------------------------------------------------------------------
     Stores the present node on store (Discard) and stores all sub-nodes prior to this
    -----------------------------------------------------------------------------*/
    virtual bool Store() const;

    /**----------------------------------------------------------------------------
     Stores point node header and points to the store. It does no provoque storage
     of sub-nodes.
     Overloads HPMStoredPooledVector::Discard()
     In addition advises parent node in case of block ID change.
    -----------------------------------------------------------------------------*/
    virtual bool Discard() const; // Intentionaly const ... only mutable members are modified

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
    virtual bool AddConditional (const POINT pi_rpSpatialObject, bool ExtentFixed);

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
    virtual size_t AddArrayConditional (const POINT* pointsArray, size_t startPointIndex, size_t countPoints, bool ExtentFixed);

    /**----------------------------------------------------------------------------
     Adds point in node.
     The point must be included (contained) in node extent

     @param Point to add
    -----------------------------------------------------------------------------*/
    virtual bool Add(const POINT pi_rpSpatialObject);


    /**----------------------------------------------------------------------------
     Clears all points of the node ... This method of the container is here overloaded
     to add the behavior of eliminating the storage space used. This is performed by
     destroying the block and letting the storage system recuperate the disk space involved.

    -----------------------------------------------------------------------------*/
    virtual void clear();


    /**----------------------------------------------------------------------------
     Clears all points that are included in given shape.

     @param pi_shapeToClear IN The shape that defines the area to clear.

     @returns The number of points removed
    -----------------------------------------------------------------------------*/
    virtual size_t Clear(HFCPtr<HVEShape> pi_shapeToClear);

    /**----------------------------------------------------------------------------
     Gets a list of objects for the node. Note that in progressive mode some
     objects for the node are located in the ancestor nodes.

     @param pio_rListOfObjects The list of objects pertaining to the node.
    -----------------------------------------------------------------------------*/
    virtual size_t GetNodeObjects(list<POINT>& pio_rListOfObjects) const;


    /**----------------------------------------------------------------------------
     This method provoques the propagation of data down immediately.
     It overloads the base class method to indicate that filtering needs to be performed.
    -----------------------------------------------------------------------------*/
    virtual void PropagateDataDownImmediately(bool propagateRecursively = true);


    /**----------------------------------------------------------------------------
     Returns the filter used for filtering the points when promoting them to upper levels.

     @return Pointer to filter or NULL if none is set.
    -----------------------------------------------------------------------------*/
    IHGFPointIndexFilter<POINT, EXTENT>*
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
    virtual void Filter();

    /**----------------------------------------------------------------------------
     Initiates a post-filtering of the node. This is called after calling the
     filtering process. The post-filtering is recursive and will call sub-nodes
     post-filtering.
    -----------------------------------------------------------------------------*/
    virtual bool PostFilter();


    virtual void ComputeObjectRelevance();

    void AdjustViewDependentMetric(HVE2DShape& pi_rConvexHull);

    double* GetViewDependentMetrics();


    /**----------------------------------------------------------------------------
     Loads the present tile if delay loaded.
    -----------------------------------------------------------------------------*/
    void Load() const; // Intentionaly const as only mutable members are modified

    /**----------------------------------------------------------------------------
    Unloads the present tile if delay loaded.
    -----------------------------------------------------------------------------*/
    void Unload() const; // Intentionaly const as only mutable members are modified

    virtual bool Destroy();


    uint32_t     GetNbObjects() const;

    uint64_t    GetNbObjectsAtLevel(size_t pi_depthLevel) const;

#ifdef __HMR_DEBUG
    void         DumpQuadTreeNode(FILE* pi_pOutputXmlFileStream,
                                  bool pi_OnlyLoadedNode) const;
#endif

    virtual void               ValidateInvariants() const
        {
#ifdef __HMR_DEBUG
        // Validate ancester invariants ...
        HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<HGFPointIndexNode<POINT, EXTENT> >, HGFPointNodeHeader<EXTENT> >::ValidateInvariants();
        if (IsLoaded() && !m_destroyed)
            {
            // Make sure the number of points is not over the split treshold (unless it is an unsplit node, unspliteable or a filtered parent of an unspliteable node)
            HASSERT (this->size() <= m_nodeHeader.m_SplitTreshold || m_nodeHeader.m_IsUnSplitSubLevel || m_unspliteable || (m_nodeHeader.m_filtered && m_parentOfAnUnspliteableNode));
            }
#endif // __HMR_DEBUG
        }

    virtual void               ValidateInvariantsSoft() const
        {
        // Validate ancester invariants ...
        HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<HGFPointIndexNode<POINT, EXTENT> >, HGFPointNodeHeader<EXTENT> >::ValidateInvariantsSoft();
        }

protected:

    /**----------------------------------------------------------------------------
     Sets the sub nodes ... this can only be for a leaf node and the subnodes must
     exist.

     @param pi_apSubNodes - Array of four sub-nodes pointer that must point to existing
                           nodes
    -----------------------------------------------------------------------------*/
    void SetSubNodes(HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pi_apSubNodes[], size_t numSubNodes);

    /**----------------------------------------------------------------------------
     Performs the pre-query process upon node and sub-nodes

     @param queryObject IN The query object to apply pre-query with.

     @return true if the process should continue and false otherwise.

    -----------------------------------------------------------------------------*/
    bool              PreQuery (IHGFPointIndexQuery<POINT, EXTENT>* queryObject);

    /**----------------------------------------------------------------------------
     Performs the query process upon node and sub-nodes

     @param queryObject IN The query object to apply query with.

     @param points IN OUT The list of ponts to which the query object may add
       points as part of the result set.

     @return true if the process should dig down into sub-nodes and false otherwise.
     TBD Something wrong with query return value.
    -----------------------------------------------------------------------------*/
    bool              Query (IHGFPointIndexQuery<POINT, EXTENT>* queryObject, list<POINT>& points);
    bool              Query (IHGFPointIndexQuery<POINT, EXTENT>* queryObject, HPMMemoryManagedVector<POINT>& points);

    /**----------------------------------------------------------------------------
     Performs the post-query process upon node and sub-nodes

     @param queryObject IN The query object to apply post-query with.

     @return true if the process should continue and false otherwise.
    -----------------------------------------------------------------------------*/
    bool              PostQuery (IHGFPointIndexQuery<POINT, EXTENT>* queryObject);

    void              AddNumberObjectsInAncestors(EXTENT& pi_rNodeExtent,
                                                  uint32_t& pio_rNbPoints) const;

    void              AddNodeObjectsInAncestors(const EXTENT& pi_rChildNodeExtent,
                                                list<POINT>&  pio_rListOfObjects) const;


    /**----------------------------------------------------------------------------
     Adds a reference to spatial object in node.
     The spatial object extent must be included (contained) in node extent

     @param Pointer to spatial object to add a reference to in the index
    -----------------------------------------------------------------------------*/
    bool AddArrayUnconditional(const POINT* pointsArray, size_t countPoints);

    /**----------------------------------------------------------------------------
     This method advises that one of the subnode storage block ID has changed
     as a result of reallocation, destruction or initial store.

     @param p_subNode - Pointer to sub node for which the ID changed
    -----------------------------------------------------------------------------*/
    virtual void AdviseSubNodeIDChanged(const HFCPtr<HGFPointIndexNode<POINT, EXTENT> >& p_subNode);



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

    IHGFPointIndexFilter<POINT, EXTENT>* m_filter;
    };
















template<class POINT, class EXTENT> class HGFPointIndex : public HGFSpatialIndex <POINT, POINT, EXTENT, HGFPointIndexNode<POINT, EXTENT>, HGFPointIndexHeader<EXTENT> >
    {

public:

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
    HGFPointIndex(HFCPtr<HPMCountLimitedPool<POINT> > pool, HFCPtr<HGFPointTileStore<POINT, EXTENT> > store, size_t SplitTreshold, IHGFPointIndexFilter<POINT, EXTENT>* filter, bool balanced, bool propagatesDataDown);
    /**----------------------------------------------------------------------------
     Destructor
     If the index has unstored nodes then those will be stored.
    -----------------------------------------------------------------------------*/
    virtual             ~HGFPointIndex();

    /**----------------------------------------------------------------------------
     Returns the meory pool
    -----------------------------------------------------------------------------*/
    HFCPtr<HPMCountLimitedPool<POINT> >
    GetPool() const;

    /**----------------------------------------------------------------------------
     Returns the store
    -----------------------------------------------------------------------------*/
    HFCPtr<HGFPointTileStore<POINT, EXTENT> >
    GetStore() const;

    /**----------------------------------------------------------------------------
     Forces an immmediate store (to minimize the chances of corruption
    -----------------------------------------------------------------------------*/
    virtual bool        Store();

    /**----------------------------------------------------------------------------
     Changes the store. The node is not destroyed from the original store, but
     a copy is performed on the new store.
    -----------------------------------------------------------------------------*/
    virtual bool        ChangeStore(HFCPtr<HGFPointTileStore<POINT, EXTENT> > newStore);

    /**----------------------------------------------------------------------------
     Returns the filter used for filtering the points when promoting them to upper levels.

     @return Pointer to filter or NULL if none is set.
    -----------------------------------------------------------------------------*/
    IHGFPointIndexFilter<POINT, EXTENT>*
    GetFilter() ;

    /**----------------------------------------------------------------------------
     Initiates the filtering of the point index, This filtering process includes
     recursively calling global pre-filtering, pre=filtering of nodes, filtering,
     node post-filtering then global post-filtering.

    -----------------------------------------------------------------------------*/
    virtual void        Filter();

    /**----------------------------------------------------------------------------
     Initiates a query upon the point index, This query process includes
     recursively calling global pre-query, pre-query of nodes, query proper,
     node post-query then global post-filtering.

    -----------------------------------------------------------------------------*/
    size_t              Query (IHGFPointIndexQuery<POINT, EXTENT>* queryObject,
                               list<POINT>& resultPoints);
    size_t              Query (IHGFPointIndexQuery<POINT, EXTENT>* queryObject,
                               HPMMemoryManagedVector<POINT>& resultPoints);

    // To be moved into filtering object
    void                ComputeObjectRelevance();

    uint64_t           GetNbObjectsAtLevel(size_t pi_depthLevel);

    // HGFSpatilaIndex implementation overrides
    bool                Add(const POINT pi_pSpatialObject);
    bool                AddArray (const POINT* pointsArray, size_t countOfPoints);
    bool                Clear(HFCPtr<HVEShape> pi_shapeToClear);
    size_t              GetAt(const POINT& pi_rCoord,
                              list<POINT>& pio_rListOfObjects) const;




#ifdef __HMR_DEBUG
    void                DumpQuadTree(char* pi_pOutputXMLFileName, bool pi_OnlyLoadedNode) const;
#endif


    void                AdjustViewDependentMetric(HVE2DShape& pi_rConvexHull);
protected:

    HGFPointIndex(const HGFPointIndex&   pi_rSpatialObject);

    HGFPointIndex&
    operator=(const HGFPointIndex& pi_rObj);


    virtual void                ValidateInvariants() const
        {
        HGFSpatialIndex <POINT, POINT, EXTENT, HGFPointIndexNode<POINT, EXTENT>, HGFPointIndexHeader<EXTENT> >::ValidateInvariants();

        };


private:
    HFCPtr<HPMCountLimitedPool<POINT> > m_pool;
    HFCPtr<HGFPointTileStore<POINT, EXTENT> > m_store;
    IHGFPointIndexFilter<POINT, EXTENT>* m_filter;
    };






/*======================================================================================================
** The IHGFPointIndexFilter provides a mechanism to implement a generic filter for the point index
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
** nodes part of the index. It is at this stage and the current HGFPointIndexFilter implementation
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

template<class POINT, class EXTENT> class IHGFPointIndexFilter
    {
public:
    IHGFPointIndexFilter() {};
    virtual ~IHGFPointIndexFilter() {};

    virtual bool        ComputeObjectRelevance (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > outputNode) = 0;

    /*======================================================================================================
    ** This property indicates if the filtering process is progressive or not. Since the main purpose of
    ** filtering is to spread spatially index points conceptually located at the leaf towards parent nodes
    ** to provide importance to these points, the filter may either copy the leaf points or move points
    ** to parent nodes. If points are moved then the filter is progressive and if they are copied then the
    ** filter is not progressive. This state is very important to understand the structure of the point
    ** distribution and essential when Query process will be performed.
    **====================================================================================================*/
    virtual bool        IsProgressiveFilter() const = 0;

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
    virtual bool        GlobalPreFilter (HGFPointIndex<POINT, EXTENT>& index) {
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
    virtual bool        PreFilter (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                                   HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes) {
        return false;
        };


    /*======================================================================================================
    ** This method re-orders or cleans the node point datasets. Both node and sub-nodes point set may and
    ** will usually be modified. The process gathers points from all sub-nodes, makes a selection
    ** of points that should be promoted to the parent level then redistributes points in the subnodes.
    **====================================================================================================*/
    virtual bool        Filter (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                                HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                                size_t numSubNodes,
                                double viewParameters[]) const = 0;

    /*======================================================================================================
    ** The filter leaf process is called for leaf nodes only. It is customary for the real filtering
    ** process to occur when filtering is applied to the parent node. The filter leaf will thus
    ** usually not perform any filtering process yet may be useful for other purposes such as pre-calculations
    ** The method will return true if any processing has been applied to the leaf node and false otherwise.
    **====================================================================================================*/
    virtual bool        FilterLeaf (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                                    double viewParameters[]) const = 0;



    /*======================================================================================================
    ** The PostFilter method is called after the filter process ... It provides a way for the filter object
    ** to clean ressources filter or format result set.
    ** Since not all filter objects need a post-filter performed a default implementation is provided.
    **
    ** @return true if the post-filter must continue and false otherwise. Even if false is returned the
    ** global post-filter process will be called.
    **====================================================================================================*/
    virtual bool        PostFilter (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
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
    virtual bool        GlobalPostFilter (HGFPointIndex<POINT, EXTENT>& index) {
        return true;
        };

    };




/*======================================================================================================
** The IHGFPointIndexQuery provides a mechanism to implement a generic query for the point index
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
** nodes part of the index. It is at this stage and the current HGFPointIndexQuery implementation
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

template<class POINT, class EXTENT> class IHGFPointIndexQuery
    {
public:
    IHGFPointIndexQuery() {};
    virtual ~IHGFPointIndexQuery() {};

    /*======================================================================================================
    ** The GlobalPreQuery method is called before the pre-query and query process ... It provides a way
    ** for the query object to allocate ressources or do whatever is requesred prior to starting the
    ** process. Global preQuery is not usually required.
    ** Since most query objects do not need a gloabl pre-query performed a default implementation is provided.
    **
    ** @return true if the query must continue and false to cancle it all together. If false neither
    ** pre-query, query or post-query will be called, but the global post-query will still be callsed
    **====================================================================================================*/
    virtual bool        GlobalPreQuery (HGFPointIndex<POINT, EXTENT>& index,
                                        list<POINT>& points) {
        return true;
        };
    virtual bool        GlobalPreQuery (HGFPointIndex<POINT, EXTENT>& index,
                                        HPMMemoryManagedVector<POINT>& points) {
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
    virtual bool        PreQuery (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                                  HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
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
    virtual bool        Query (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                               HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               list<POINT>& points) = 0;
    virtual bool        Query (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                               HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               HPMMemoryManagedVector<POINT>& points) = 0;


    /*======================================================================================================
    ** The PostQuery method is called after the query process ... It provides a way for the query object
    ** to clean ressources filter or format result set.
    ** Since not all query objects need a post-query performed a default implementation is provided.
    **
    ** @return true if the post-query must continue and false otherwise. Even if false is returned the
    ** global post-query process will be called.
    **====================================================================================================*/
    virtual bool        PostQuery (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                                   HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
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
    virtual bool        GlobalPostQuery (HGFPointIndex<POINT, EXTENT>& index,
                                         list<POINT>& points) {
        return true;
        }
    virtual bool        GlobalPostQuery (HGFPointIndex<POINT, EXTENT>& index,
                                         HPMMemoryManagedVector<POINT>& points) {
        return true;
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
** The IHGFPointIndexSpatialLimitWrapQuery provides a mechanism to implement a generic query for the point index
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

template<class POINT, class EXTENT> class IHGFPointIndexSpatialLimitWrapQuery : public IHGFPointIndexQuery<POINT, EXTENT>
    {
private:
    HFCPtr<HVE2DShape> m_limit;
    IHGFPointIndexQuery* m_wrappedQuery;

    bool IsNodeDisjointFromLimit (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node)
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
    IHGFPointIndexSpatialLimitWrapQuery(HFCPtr<HVE2DShape> pi_Limit, IHGFPointIndexQuery* pi_Query)
        {
        m_limit = pi_Limit;
        m_wrappedQuery = pi_Query;
        }
    virtual ~IHGFPointIndexSpatialLimitWrapQuery()
        {
        delete m_wrappedQuery;
        }


    /*======================================================================================================
    ** @return Returns the wrapped query
    **====================================================================================================*/
    IHGFPointIndexQuery* GetWrappedQuery() const {
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
    virtual bool        PreQuery (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                                  HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
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
    virtual bool        Query (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                               HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               list<POINT>& points)
        {
        // Check if node is within limit
        if (IsNodeDisjointFromLimit (node))
            {
            // Shapes are disjoint
            return false;
            }

        list<POINT> tempList;

        // Called wrapped query
        bool status =  m_wrappedQuery->Query(node, subNodes, numSubNodes, tempList);

        list<POINT>::iterator itrPts;
        for (itrPts = tempList.begin() ; itrPts != tempList.end() ; itrPts++)
            {
            if (m_limit->IsPointIn(HGF2DLocation(PointOp<POINT>::GetX(*itrPts), PointOp<POINT>::GetY(*itrPts), m_limit->GetCoordSys())))
                points.push_back(*itrPts);
            }

        return status;

        }

    virtual bool        Query (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                               HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               HPMMemoryManagedVector<POINT>& points)
        {
        // Check if node is within limit
        if (IsNodeDisjointFromLimit (node))
            {
            // Shapes are disjoint
            return false;
            }

        list<POINT> tempList;

        // Called wrapped query
        bool status =  m_wrappedQuery->Query(node, subNodes, numSubNodes, tempList);

        list<POINT>::iterator itrPts;
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
    virtual bool        PostQuery (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                                   HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
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
template<class POINT, class EXTENT> class HGFLevelPointIndexQuery: public IHGFPointIndexQuery<POINT, EXTENT>
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

    // The global prequery simply cleans pre-query specific structure
    virtual bool        GlobalPreQuery (HGFPointIndex<POINT, EXTENT>& index,
                                        list<POINT>& points)
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

    virtual bool        GlobalPreQuery (HGFPointIndex<POINT, EXTENT>& index,
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
    virtual bool        Query (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                               HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               list<POINT>& resultPoints)
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
                    for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(node->operator[](currentIndex));
                        }
                    }
                else
                    {
                    // Search in present list of objects for current node
                    for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                        {
                        // Check if point is in extent of object
                        if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, node->operator[](currentIndex)))
                            {
                            // The point falls inside extent of object .. we add a reference to the list
                            resultPoints.push_back(node->operator[](currentIndex));
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
    virtual bool        Query (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                               HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
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
                    for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(node->operator[](currentIndex));
                        }
                    }
                else
                    {
                    // Search in present list of objects for current node
                    for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                        {
                        // Check if point is in extent of object
                        if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, node->operator[](currentIndex)))
                            {
                            // The point falls inside extent of object .. we add a reference to the list
                            resultPoints.push_back(node->operator[](currentIndex));
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
template<class POINT, class EXTENT> class HGFLevelPointIndexByShapeQuery: public IHGFPointIndexQuery<POINT, EXTENT>
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

    // The global prequery simply cleans pre-query specific structure
    virtual bool        GlobalPreQuery (HGFPointIndex<POINT, EXTENT>& index,
        list<POINT>& points) 
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

    virtual bool        GlobalPreQuery (HGFPointIndex<POINT, EXTENT>& index,
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
    virtual bool        Query (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node, 
        HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
        size_t numSubNodes,
        list<POINT>& resultPoints)
        {        
        assert("Not implemented yet!");                
        return false;
        }
    virtual bool        Query (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node, 
        HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
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
                    for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(node->operator[](currentIndex));
                        }
                    }
                else
                    {                    
                    // Search in present list of objects for current node
                    for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                        {
                        // Check if point is in extent of object
                        if (m_areaShape->IsPointIn(HGF2DLocation(node->operator[](currentIndex).x, node->operator[](currentIndex).y, m_areaShape->GetCoordSys())))
                            {
                            // The point falls inside extent of object .. we add a reference to the list
                            resultPoints.push_back(node->operator[](currentIndex));
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
    virtual bool        GlobalPreQuery (HGFPointIndex<POINT, EXTENT>& index,
                                        list<POINT>& points)
        {
        m_levelSet = false;

        return true;
        };
    virtual bool        GlobalPreQuery (HGFPointIndex<POINT, EXTENT>& index,
                                        HPMMemoryManagedVector<POINT>& points)
        {
        m_levelSet = false;

        return true;
        };



    // The pre-query will determine the depth level to perform the query upon
    virtual bool        PreQuery (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                                  HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
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
template<class POINT, class EXTENT> class HGFNeighborNodesPointIndexQuery: public IHGFPointIndexQuery<POINT, EXTENT>
    {
private:
    HGFPointIndex<POINT, EXTENT>*  m_index;
    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  m_node;
    list <HFCPtr<HGFPointIndexNode<POINT, EXTENT> > > m_neighborNodes;
public:

    HGFNeighborNodesPointIndexQuery(HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node)
        {
        m_node = node;
        m_index = NULL;
        }
    virtual ~HGFNeighborNodesPointIndexQuery()
        {
        m_index = NULL;
        m_node = NULL;
        }

    const list<HFCPtr<HGFPointIndexNode<POINT, EXTENT> > >&
    GetResultNeighborList() const
        {
        return m_neighborNodes;
        }

    // The global prequery simply cleans pre-query specific structure
    virtual bool        GlobalPreQuery (HGFPointIndex<POINT, EXTENT>& index,
                                        list<POINT>& points)
        {
        // Save the index
        m_index = &index;
        m_neighborNodes.clear();
        return true;
        };
    virtual bool        GlobalPreQuery (HGFPointIndex<POINT, EXTENT>& index,
                                        HPMMemoryManagedVector<POINT>& points)
        {
        // Save the index
        m_index = &index;
        m_neighborNodes.clear();
        return true;
        };

    // The Query process gathers points up to level depth
    virtual bool        Query (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                               HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               list<POINT>& resultPoints)
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
    virtual bool        Query (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                               HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
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
    virtual bool        GlobalPostQuery (HGFPointIndex<POINT, EXTENT>& index,
                                         list<POINT>& points)
        {
        // Save the index
        list<HFCPtr<HGFPointIndexNode<POINT, EXTENT>>>::iterator nodeIter(m_neighborNodes.begin());
        list<HFCPtr<HGFPointIndexNode<POINT, EXTENT>>>::iterator nodeIterEnd(m_neighborNodes.end());

        EXTENT inflatedNodeExtent = ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_node->GetNodeExtent()) - ExtentOp<EXTENT>::GetWidth(m_node->GetNodeExtent()) * 0.05,
                                                             ExtentOp<EXTENT>::GetYMin(m_node->GetNodeExtent()) - ExtentOp<EXTENT>::GetHeight(m_node->GetNodeExtent()) * 0.05,
                                                             ExtentOp<EXTENT>::GetXMax(m_node->GetNodeExtent()) + ExtentOp<EXTENT>::GetWidth(m_node->GetNodeExtent()) * 0.05,
                                                             ExtentOp<EXTENT>::GetYMax(m_node->GetNodeExtent()) + ExtentOp<EXTENT>::GetHeight(m_node->GetNodeExtent()) * 0.05);

        while (nodeIter != nodeIterEnd)
            {
            for (uint32_t PtInd = 0; PtInd < (*nodeIter)->size(); PtInd++)
                {
                if (ExtentOp<EXTENT>::InnerOverlap((*nodeIter)->GetNodeExtent(), inflatedNodeExtent) == false)
                    continue;

                POINT point = (*nodeIter)->operator[](PtInd);

                if (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent2D(point, inflatedNodeExtent))
                    {
                    points.push_back(point);
                    }
                }

            nodeIter++;
            }


        /*
        while (nodeIter != nodeIterEnd)
        {
            tempListOfObjects.clear();

            (*nodeIter)->GetNodeObjects(tempListOfObjects);

            objIter = tempListOfObjects.begin();
            objIterEnd = tempListOfObjects.end();

            while (objIter != objIterEnd)
            {
                if (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent2D(*objIter, inflatedNodeExtent))
                {
                    points.push_back(*objIter);
                }
                objIter++;
            }
            nodeIter++;
        }
        */

        m_index = NULL;
        return true;
        };
    // The global post query releases the index pointer
    virtual bool        GlobalPostQuery (HGFPointIndex<POINT, EXTENT>& index,
                                         HPMMemoryManagedVector<POINT>& points)
        {
        // Save the index
        list<HFCPtr<HGFPointIndexNode<POINT, EXTENT>>>::iterator nodeIter(m_neighborNodes.begin());
        list<HFCPtr<HGFPointIndexNode<POINT, EXTENT>>>::iterator nodeIterEnd(m_neighborNodes.end());

        EXTENT inflatedNodeExtent = ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_node->GetNodeExtent()) - ExtentOp<EXTENT>::GetWidth(m_node->GetNodeExtent()) * 0.05,
                                                             ExtentOp<EXTENT>::GetYMin(m_node->GetNodeExtent()) - ExtentOp<EXTENT>::GetHeight(m_node->GetNodeExtent()) * 0.05,
                                                             ExtentOp<EXTENT>::GetXMax(m_node->GetNodeExtent()) + ExtentOp<EXTENT>::GetWidth(m_node->GetNodeExtent()) * 0.05,
                                                             ExtentOp<EXTENT>::GetYMax(m_node->GetNodeExtent()) + ExtentOp<EXTENT>::GetHeight(m_node->GetNodeExtent()) * 0.05);

        while (nodeIter != nodeIterEnd)
            {
            for (uint32_t PtInd = 0; PtInd < (*nodeIter)->size(); PtInd++)
                {
                if (ExtentOp<EXTENT>::InnerOverlap((*nodeIter)->GetNodeExtent(), inflatedNodeExtent) == false)
                    continue;

                POINT point = (*nodeIter)->operator[](PtInd);

                if (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent2D(point, inflatedNodeExtent))
                    {
                    if (points.size() == points.capacity())
                        points.reserve(points.size() + (points.size()/10) + 1);
                    points.push_back(point);
                    }
                }

            nodeIter++;
            }


        /*
        while (nodeIter != nodeIterEnd)
        {
            tempListOfObjects.clear();

            (*nodeIter)->GetNodeObjects(tempListOfObjects);

            objIter = tempListOfObjects.begin();
            objIterEnd = tempListOfObjects.end();

            while (objIter != objIterEnd)
            {
                if (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent2D(*objIter, inflatedNodeExtent))
                {
                    points.push_back(*objIter);
                }
                objIter++;
            }
            nodeIter++;
        }
        */

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
template<class POINT, class EXTENT> class HGFViewDependentPointIndexQuery: public IHGFPointIndexQuery<POINT, EXTENT>
    {
private:

    list<HVE2DSegment> m_listOfTileBreaklines;

protected :

    EXTENT m_extent;
    bool   m_gatherTileBreaklines;
    int    m_nearestPredefinedCameraOri;
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
        m_nearestPredefinedCameraOri;
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
    virtual bool        GlobalPreQuery (HGFPointIndex<POINT, EXTENT>& index,
                                        list<POINT>& points)
        {
        m_listOfTileBreaklines.clear();

        bcdtmMultiResolution_getSampleCameraOri(m_viewportRotMatrix, &m_nearestPredefinedCameraOri);

        return true;
        };
    virtual bool        GlobalPreQuery (HGFPointIndex<POINT, EXTENT>& index,
                                        HPMMemoryManagedVector<POINT>& points)
        {
        m_listOfTileBreaklines.clear();

        bcdtmMultiResolution_getSampleCameraOri(m_viewportRotMatrix, &m_nearestPredefinedCameraOri);

        return true;
        };


    // Specific Query implementation
    virtual bool        Query (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                               HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               list<POINT>& resultPoints)
        {
#if (0)
        // Check if extent overlap
        if (!ExtentOp<EXTENT>::Overlap(node->GetNodeExtent(), m_extent))
            return false;

        bool finalNode = false;
        // Determine if it is the final node ...
        if (!node->IsLeaf())
            {

            // We need to determine if we need to go deeper ...
            for (size_t indexSubNodes = 0 ; !finalNode && indexSubNodes < numSubNodes ; indexSubNodes++)
                {
                if (subNodes[indexSubNodes] != NULL)
                    {
                    if ((subNodes[indexSubNodes]->size() > 0) &&
                        (ExtentOp<EXTENT>::Overlap(m_extent, subNodes[indexSubNodes]->GetNodeExtent())))
                        finalNode = finalNode || IsCorrectForCurrentView(subNodes[indexSubNodes], m_nearestPredefinedCameraOri, m_rootToViewMatrix);
                    }
                }
            }


        // Check if coordinate falls inside node extent
        if (IsCorrectForCurrentView(node, m_nearestPredefinedCameraOri, m_rootToViewMatrix))
            {
            // The point is located inside the node ...
            // Obtain objects from subnodes (if any)
            if (node->IsLeaf() || node->GetFilter()->IsProgressiveFilter() || finalNode)
                {
                for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                    {
                    // Check if point is in extent of object
                    if (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent2D((node->operator[](currentIndex)), m_extent))
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(node->operator[](currentIndex));
                        }
                    }
                if (m_gatherTileBreaklines && node->size() > 0)
                    {
                    AddBreaklinesForExtent(node->GetNodeExtent());
                    }
                }
            }

        return (!finalNode);
#else
        // Check if extent overlap
        if (!ExtentOp<EXTENT>::Overlap(node->GetNodeExtent(), m_extent))
            return false;

        bool finalNode = false;

        // Check if coordinate falls inside node extent
        finalNode = !IsCorrectForCurrentView(node, node->GetNodeExtent(), m_nearestPredefinedCameraOri, m_rootToViewMatrix);

        // The point is located inside the node ...
        // Obtain objects from subnodes (if any)
        if (finalNode == false)
            {
            if (node->GetFilter()->IsProgressiveFilter())
                {
                for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                    {
                    // Check if point is in extent of object
                    if ((node->GetFilter()->IsProgressiveFilter() && (node->GetLevel() == 0)) ||
                        ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, (node->operator[](currentIndex))))
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(node->operator[](currentIndex));
                        }
                    }
                }
            }
        else
            {
            if ((node->GetParentNode() != 0) &&
                (node->GetFilter()->IsProgressiveFilter() == false))
                {
                HFCPtr<HGFPointIndexNode<POINT, EXTENT>> parentNode = node->GetParentNode();

                for (size_t currentIndex = 0 ; currentIndex < parentNode->size(); currentIndex++)
                    {
                    // Check if point is in extent of object
                    if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, parentNode->operator[](currentIndex)))
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(parentNode->operator[](currentIndex));
                        }
                    }
                }
            }

        if (finalNode && m_gatherTileBreaklines && node->size() > 0)
            {
            AddBreaklinesForExtent(node->GetNodeExtent());
            }

        return (!finalNode);
#endif
        }
    virtual bool        Query (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                               HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               HPMMemoryManagedVector<POINT>& resultPoints)
        {
#if (0)
        // Check if extent overlap
        if (!ExtentOp<EXTENT>::Overlap(node->GetNodeExtent(), m_extent))
            return false;

        bool finalNode = false;
        // Determine if it is the final node ...
        if (!node->IsLeaf())
            {

            // We need to determine if we need to go deeper ...
            for (size_t indexSubNodes = 0 ; !finalNode && indexSubNodes < numSubNodes ; indexSubNodes++)
                {
                if (subNodes[indexSubNodes] != NULL)
                    {
                    if ((subNodes[indexSubNodes]->size() > 0) &&
                        (ExtentOp<EXTENT>::Overlap(m_extent, subNodes[indexSubNodes]->GetNodeExtent())))
                        finalNode = finalNode || IsCorrectForCurrentView(subNodes[indexSubNodes], subNodes[indexSubNodes]->GetNodeExtent(), m_nearestPredefinedCameraOri, m_rootToViewMatrix);
                    }
                }
            }


        // Check if coordinate falls inside node extent
        if (IsCorrectForCurrentView(node, m_nearestPredefinedCameraOri, m_rootToViewMatrix))
            {
            // The point is located inside the node ...
            // Obtain objects from subnodes (if any)
            if (node->IsLeaf() || node->GetFilter()->IsProgressiveFilter() || finalNode)
                {
                for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                    {
                    // Check if point is in extent of object
                    if (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent2D((node->operator[](currentIndex)), m_extent))
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(node->operator[](currentIndex));
                        }
                    }
                if (m_gatherTileBreaklines && node->size() > 0)
                    {
                    AddBreaklinesForExtent(node->GetNodeExtent());
                    }
                }
            }

        return (!finalNode);
#else
        // Check if extent overlap
        if (!ExtentOp<EXTENT>::Overlap(node->GetNodeExtent(), m_extent))
            return false;

        bool finalNode = false;

        // Check if coordinate falls inside node extent
        finalNode = !IsCorrectForCurrentView(node, node->GetNodeExtent(), m_nearestPredefinedCameraOri, m_rootToViewMatrix);

        // The point is located inside the node ...
        // Obtain objects from subnodes (if any)
        if (finalNode == false)
            {
            if (node->GetFilter()->IsProgressiveFilter())
                {
                for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                    {
                    // Check if point is in extent of object
                    if ((node->GetFilter()->IsProgressiveFilter() && (node->GetLevel() == 0)) ||
                        ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, (node->operator[](currentIndex))))
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(node->operator[](currentIndex));
                        }
                    }
                }
            }
        else
            {
            if ((node->GetParentNode() != 0) &&
                (node->GetFilter()->IsProgressiveFilter() == false))
                {
                HFCPtr<HGFPointIndexNode<POINT, EXTENT>> parentNode = node->GetParentNode();

                for (size_t currentIndex = 0 ; currentIndex < parentNode->size(); currentIndex++)
                    {
                    // Check if point is in extent of object
                    if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, parentNode->operator[](currentIndex)))
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(parentNode->operator[](currentIndex));
                        }
                    }
                }
            }

        if (finalNode && m_gatherTileBreaklines && node->size() > 0)
            {
            AddBreaklinesForExtent(node->GetNodeExtent());
            }

        return (!finalNode);
#endif
        }


    virtual bool IsCorrectForCurrentView(HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                                         const EXTENT& pi_visibleExtent,
                                         int           pi_NearestPredefinedCameraOri,
                                         double        pi_RootToViewMatrix[][4]) const = 0;
    };


/** -----------------------------------------------------------------------------

    This class implements a default filter for spatial index of points. It takes
    one every four points out of every vectors.
    -----------------------------------------------------------------------------
*/
template<class POINT, class EXTENT> class HGFPointIndexDefaultFilter : public IHGFPointIndexFilter<POINT, EXTENT>
    {

public:

    // Primary methods
    HGFPointIndexDefaultFilter() {};
    virtual             ~HGFPointIndexDefaultFilter() {};

    // IHGFPointFilter implementation
    virtual bool        Filter (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode,
                                HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                                size_t numSubNodes,
                                double viewParameters[]) const
        {
        size_t totalNumPoints = 0;
        for (size_t indexNode = 1; indexNode < numSubNodes; indexNode++)
            {
            if (subNodes[indexNode] != NULL)
                {
                totalNumPoints += subNodes[indexNode]->size();
                }
            }


        parentNode->reserve (totalSubNodes / numSubNodes + 1);
        for (size_t indexNode = 1; indexNode < numSubNodes; indexNode++)
            {
            if (subNodes[indexNode] != NULL)
                {
                for (size_t indexPoint = 0 ; indexPoint < subNodes[indexNode]->size() ; indexPoint += numSubNodes)
                    parentNode->push_back (subNodes[indexNode]->operator[](indexPoint));
                }
            }

        return true;
        }




    virtual bool        FilterLeaf (HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  outputNode,
                                    double viewParameters[]) const
        {
        return true;
        }

    virtual bool        IsProgressiveFilter() const {
        return false;
        };
    };



#if (1)
// TBD To remove
template <class POINT, class EXTENT> HGFPointIndex<POINT, EXTENT>& operator<<(HGFPointIndex<POINT, EXTENT>& mrDTM, const char* XYZFileName)
    {
    double xyz[3001];
    POINT myArrayOfPoints[1000];
    size_t indexOfPoints;
    // Open file
    FILE* theFile = fopen (XYZFileName, "rb");
    size_t count = 0;
    size_t lastCount = 0;

    if (theFile != NULL)
        {
        indexOfPoints = 0;
        while (!feof(theFile))
            {
            indexOfPoints = fread (xyz, 3*sizeof(double), 1000, theFile);
            if (indexOfPoints >= 0)
                {


                for (size_t theIndex = 0 ; theIndex < indexOfPoints; ++theIndex)
                    {
                    myArrayOfPoints[theIndex] = PointOp<POINT>::Create(xyz[3*theIndex], xyz[3*theIndex+1], xyz[3*theIndex+2]);
                    }
                mrDTM.AddArray (myArrayOfPoints, indexOfPoints);
                count += indexOfPoints;

                if (count - lastCount > 10000)
                    {
                    std::cout << count << std::endl;
                    lastCount = count;
                    }

                }
            }


        fclose (theFile);

        }
    return mrDTM;

    }

template<class POINT, class EXTENT> int LoadDTMFromXYZMultipleTimesWithOffset(HGFPointIndex<POINT, EXTENT>& mrDTM, char* XYZFileName, int numX, int numY)
    {
    // Load first time without offset
    size_t count = LoadDTMFromXYZWithOffset (mrDTM, XYZFileName, 0.0, 0.0, 0);

//     size_t count = 0;

    // Obtain the extent of result mrDTM
    EXTENT myInitialExtent = mrDTM.GetContentExtent();

    double xMin = myInitialExtent.GetXMin();
    double xMax = myInitialExtent.GetXMax();
    double yMin = myInitialExtent.GetYMin();
    double yMax = myInitialExtent.GetYMax();

    for (size_t xIndex = 0; xIndex < numX; xIndex++)
        {
        for (size_t yIndex = 0; yIndex < numY; yIndex++)
            {
            if ((xIndex != 0) || (yIndex != 0))
                {
                count = LoadDTMFromXYZWithOffset (mrDTM, XYZFileName, xIndex * (xMax - xMin), yIndex * (yMax - yMin), count);

                }
            }
        }
    return 0;
    }



template<class POINT, class EXTENT> size_t LoadDTMFromXYZWithOffset(HGFPointIndex<POINT, EXTENT>& mrDTM, char* XYZFileName, double xOffset, double yOffset, size_t countOffset)
    {
    double xyz[3001];
    POINT myArrayOfPoints[1000];
    size_t indexOfPoints;
    // Open file
    FILE* theFile = fopen (XYZFileName, "rb");
    size_t count = countOffset;
    size_t lastCount = countOffset;

    if (theFile != NULL)
        {
        indexOfPoints = 0;
        while (!feof(theFile))
            {
            indexOfPoints = fread (xyz, 3*sizeof(double), 1000, theFile);
            if (indexOfPoints >= 0)
                {


                for (size_t theIndex = 0 ; theIndex < indexOfPoints; ++theIndex)
                    {
                    myArrayOfPoints[theIndex] = POINT (xyz[3*theIndex] + xOffset, xyz[3*theIndex+1] + yOffset, xyz[3*theIndex+2]);
                    }
                mrDTM.AddPointsArray (myArrayOfPoints, indexOfPoints);
                count += indexOfPoints;

                if (count - lastCount > 100000)
                    {
                    std::cout << count << std::endl;
                    lastCount = count;

                    }

                }
            }


        fclose (theFile);

        }
    return count;

    }

#endif


#if (0)
void ConvertPODTextFileToXYZ(char* PODTextFileName, char* XYZFileName)
    {

    size_t indexOfPoints;
    // Open file
    FILE* theInFile = fopen (PODTextFileName, "rt");
    FILE* theOutFile = fopen (XYZFileName, "wb");
    size_t count = 0;
    size_t lastCount = 0;
    char stringx[100];
    char stringy[100];
    char stringz[100];
    char duma[100];
    char dumb[100];
    char dumc[100];
    char dumd[100];

    if ((theInFile != NULL) && (theOutFile))
        {
        double x, y, z;
        indexOfPoints = 0;

        while (!feof(theInFile))
            {

            if (7 == fscanf (theInFile, "%s %s %s %s %s %s %s\n", stringx, stringy, stringz, duma, dumb, dumc, dumd))
                {
                x = atof(stringx);
                y = atof(stringy);
                z = atof(stringz);
                fwrite(&x, sizeof(double), 1, theOutFile);
                fwrite(&y, sizeof(double), 1, theOutFile);
                fwrite(&z, sizeof(double), 1, theOutFile);
                }
            }
        fclose (theInFile);
        fclose (theOutFile);

        }
    }
#endif

#include "HGFPointIndex.hpp"