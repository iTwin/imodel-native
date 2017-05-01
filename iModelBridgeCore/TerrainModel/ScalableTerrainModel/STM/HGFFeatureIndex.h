//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/STM/HGFFeatureIndex.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once

#include <ImagePP/all/h/HFCPtr.h>

#include <ImagePP/all/h/HGF3DExtent.h>
#include <STMInternal/Storage/IDTMTypes.h>
#include <STMInternal/Storage/IDTMFile.h>

#include <ImagePP/all/h/HPMPooledVector.h>

#include <ImagePP/all/h/HGF3DCoord.h>
#include <ImagePP/all/h/HGF2DTemplateExtent.h>
#include <ImagePP/all/h/HVE2DSegment.h>
#include "HGFSpatialIndex.h"
#include "HGFPointTileStore.h"


// Predeclaration of the Feature Index Filter interface. This interface is defined lower in this same file.
template<class POINT, class EXTENT> class IHGFFeatureIndexFilter;

/** -----------------------------------------------------------------------------

    The following classes implement a feature index. The index is based on a quadtree
    (or octtree) structure. The architecture of the index makes it so that a single
    function needs to be overloaded in order to change the spliting algorithm that
    ultimately dictates if the spatial index has 4 or 8 sub-nodes and what is their
    location into space.

    In order to be the most general, the feature container type is undecided, yet this
    container must comply with a simplified STL std::vector interface a few of these
    function can be overriden if needed.

    A few flavors of the index are already provided, the most notable of which is the
    stored feature index. The type of the store is implementation specific but must comply
    with the IDataStore interface for the DataType specific to the feature.

    The feature index struture derives from a prototype specifically designed for managing
    DTM (Digital Terrain Model) features which usually contain 2.5D set of features.
    The feature index structure recognises the necessity of providing various options
    for various needs. The base structure of the index is a quadtree and it can be
    configured and used as a plain quadtree, where the features indexed will always be
    stored at a leaf of the quadtree index. The feature index however does provide a
    filtering mechansim that allows to position features at various non-leaf levels
    in the quadtree to increase or diminish the importance or accessibility of some features.

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
    The filtering process is free to remove features from any of the nodes provided (either parent or subnodes).

    A typical use of filtering would be to remove duplicate features, or to duplicate features
    we want to move to parent nodes based on whatever criterium.

    The filter must annouce right away wether it will or will not duplicate features during the filtering
    process. If features are not duplicated, and features are simply given more spatial importance
    by moving them to parent nodes, then the filtered is said to be PROGRESSIVE. If features are
    duplicated then the filter is not progressive. This progressiveness will be important during
    spatial query process.


    INDEX STRUCTURE
    The index structure is based mainly on the quadtree architecture, however some configurations
    change this structure in some case. When the index is populated with features, the
    features fill index nodes. Initially there is a single node which gets filled and
    the extent of the node content increases adequately to include features added. The extent
    is maintained square to prevent the construction of long and thin node extents that could result from
    ordered features sets provided as strip (such as resulting from reading a stripped DEM raster elevation
    file). When the number of features in the node reaches some split treshold amount of features,
    the node is split and the features of the split node are distributed to the created sub-nodes, initially
    completely emptying the split node (subsequent filtering may repopulate back the new parent node eventually).
    Once the alignement is set and the root node has split, it is still possible to add features outside
    the root node extent, but this will simply trigger the creation of a new root node and the initial root node
    will become one of the subnodes. The new root extent alignement is based upon the initial root extent
    and the location depend on the feature being added.

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
    will be the same for the whole tree, regardless of the density of features and the split treshold. This means that
    if a leaf is being split as a result of containing more features than allowed by the split treshold
    then it will first split, then propagate a message through all the index nodes about the final
    depth required. All leaf nodes that do not satisfy this depth level will increase their depth typically through splitting.

    The concept of index balancing result from the necessity to homogenize feature density or feature filtering
    density across a same level. This does not necessarily mean that the amount of features between nodes of a same
    level are similar, on the contrary. It means that the feature density of a node can be related by a predictable
    rule to the density of the leaf nodes after filtering. For example, assuming the filtering process will takes all
    features of a sub-node and populate the parent node with 1 node out of every four features in the subnodes. If the
    subnodes have high density compared to other sub-nodes of the same level then the parent node will also be high density relatively
    to the nodes at this same parent node level.

    Splitting nodes in 4 sub-nodes just to increase the level depth, is not an economical
    structure in matter of features per node, for this reason the index will increase level by simply
    creating a single and unique subnode. In this case the index remains balanced yet deviates significantly from the
    normal quadtree structure. The result greatly minimizes the amount of nodes in the index yet provides the same
    spatial indexing and balancing capabilities. It is the optimal structure if index balancing is required.
    Given this fact, the filtering process must be implemented in such a way as to filter nodes that have 4 sub-nodes or a single sub-node.
    The number of sub-nodes will be provided to the filtering process, and the filter can assume that if the number is 1 then
    the sub-node was created for level balancing purposes. Note that as the index is populated, it may happen
    that a leaf node that happens to be the single sub-node of a node will have to split as the amount of
    features has reached the split treshold. In this case, the node will NOT be split, It will instead trigger and
    event to its parent node, for this parent node to be split instead. If this occurs then the single-sub-node will be replaced
    by 4 sub-nodes. Since this process will occur during feature addition, the process may be temporarily delayed
    after the addition process has completed, resulting in a node having momentarily more features than normally allowed.
    Notice that if the single node subdivision is many levels deep, the split process will always occur at the most
    rootward node that has not been split.

    MEMORY MANAGEMENT

    The index allows for optional memory management mainly but not exclusively through the feature container.
    Typical use, will require the indexing a thousand, millions or even billions of features. Not all
    features can then be maintained in memory. An optional feature container called
    HPMStoredPooledVector provides pooled memory management. The principle is that this
    vector memory location is given to the management of a pool. Usually all node features
    container will share the same pool (or node features container from many feature indexes).
    Whenever features are needed, the pool will be asked for memory. If the maximum number of
    features is attained, before allocating required memory, it will first ask the least recently
    used item (the feature container) to discard its memory. The details of the discard process are
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
    features are loaded back from the store.

    PERMANENT STORAGE AND DATA RETRIEVAL

    In the above memory management description, everything is kept under the control of the
    feature container alone and the index must not bother about the fact memory management occurs. However,
    the storage of the features alone is not sufficient to insure complete information storage. The index and the
    index nodes contain information that must also be stored along the features in the case permanent storage is needed.
    Such information include
    the node extent, the sub-node identification, and any additional control data required. For this reason a stored index implementation
    is provided. This stored index requires and knowns some details about the store that may be used by the
    feature container. In this case, whenever data is stored, the stored index nodes will also be called and be given
    the chance to store additional data related to the node. Likewise the index master information will
    also be able to store its information as well at the end of the process or as a result of an explicit storage request.

    A stored index structure will make exclusive use of stored index nodes which are descendants to the ancester
    feature node index.

    When permanent storage of the index is involved, the index may be initially fully stored and no information
    initially loaded. When the index is created by providing a store, the index will immediately try to
    load the master control information and the root node control information. No other information will be loaded.
    When a node is accessed then the node control information (extent, number of sub-nodes ...) is loaded
    from the store. The features are not initally loaded unless required. The node and index control information
    will never be completely discarded but will remain in memory for the life of the index. Once features are accessed, then
    feature load will occur.

    The stored index will be able to make use of any IHGFFeatureIndexStore compliant object regardless of actual
    implementation. It will however recognise the fact some storage can be slow to operate. For example, the
    data could be stored on a remote file, or in a remote database. In such case data retrieval
    can be a lenghty process. The index will allow, in very specific case, not to wait for this data retieval
    process to conclude before returning control back to the caller. This will occur mainly when feature
    query is performed. If the data is not readily available, then the query request will be delayed till the
    data arrives from the store. Meanwhile the caller will continue operation. When the data finally arrives,
    then the query process will complete and the initial requester of the data will be advised the query result is completed.

    INDEX MODIFICATION

    The index implementation was initally based on the assumption that no data modification will occur
    except for the addition of new features. Of course this assumption can only be considered temporary as the purpose of have data
    to manage is to manage this data. Feature addition and removal, as well as modification will occur. For this
    reason the index provides a full interface for such operation. As modification of data (including feature addition) may
    have a significant impact on the data after the filtering process has been performed, some possible
    configurations were also provided for this purpose. The default behavior will be that the filtering process
    will not occur unless the filtering is required. If a node, or one of its sub-nodes (or sub-node to subnode ...) has been
    spatially modified, then the node will be tagged as Needing Filtering. This will not result in filtering process
    being performed anew. Instead, the filtering process will be delayed till the filtering is required.

    The factors that will trigger filtering are various. The first is that the node features is
    accessed and that it needs filtering. Notice that needing filtering results from the node content having been
    modified or one of its sub-nodes needing filtering (recursively). The next depends on the
    filter. Some filtering scheme will tolerate nicely that a node be moved if it stays
    inside the same node. If the filtering considers no spatial relation with the weight or importance
    of the feature, then moving it within the node has little importance. Another filtering process
    may be fussy enough that modifying less that 10% of the features has little impact. The same
    can hold for feature removal.

    -----------------------------------------------------------------------------
*/
template<class FEATURE, class POINT, class EXTENT> class IHGFFeatureIndexQuery;


template<class SPATIAL, class POINT, class EXTENT> class HGFFeatureIndex;





/** -----------------------------------------------------------------------------

    This class implements a feature spatial index  node.
    In addition to being a spatial index node, it implements the concept of
    storage of spatial objects

    -----------------------------------------------------------------------------
*/
template <class FEATURE, class POINT, class EXTENT> class HGFFeatureIndexNode : public HGFIndexNode<FEATURE, POINT, EXTENT, vector<FEATURE>, HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >, HGFFeatureNodeHeader<EXTENT> >
    {


    friend class HGFFeatureIndex<FEATURE, POINT, EXTENT>;

    // That is not an error ... we make the ancester class a friend to self class so that the ancester can call protected overriden virtual methods
    // of the present class ... certainly very weird but valid nevertheless
    friend class HGFIndexNode<FEATURE, POINT, EXTENT, vector<FEATURE>, HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >, HGFFeatureNodeHeader<EXTENT> >;
public:

    /**----------------------------------------------------------------------------
     Constructor for node based upon extent only.

     @param pi_SplitTreshold - The split treshold value used to determine when a
                               leaf node must be split. It is the maximum number of
                               objects referenced by node before a split occurs.

     @param pi_rExtent - The extent of the node

     @param store IN The store to which will be stored the features.

     @param balanced IN If true then the index / index node will be balanced.
    -----------------------------------------------------------------------------*/
    HGFFeatureIndexNode<FEATURE, POINT, EXTENT>(size_t pi_SplitTreshold,
                                                const EXTENT& pi_rExtent,
                                                HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> > store,
                                                bool balanced);

    /**----------------------------------------------------------------------------
     Constructor for node based upon parent node and extent.

     @param pi_SplitTreshold - The split treshold value used to determine when a
                              leaf node must be split. It is the maximum number of
                              objects referenced by node before a split occurs.

     @param pi_rExtent - The extent of the node

     @param pi_rpParentNode - Reference to pointer to parent node for created node
    -----------------------------------------------------------------------------*/
    HGFFeatureIndexNode<FEATURE, POINT, EXTENT> (size_t pi_SplitTreshold,
                                                 const EXTENT& pi_rExtent,
                                                 const HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >& pi_rpParentNode);

    /**----------------------------------------------------------------------------
     Constructor for node based upon parent node and extent. This constructor is provided
     for the explicit construction of an unsplit child for balanced indexes. If true is
     indicated in the last parameters it will simply consider that it cannot split when
     the split treshold is attained and will instead dispatch a message to the parent.


     @param pi_SplitTreshold - The split treshold value used to determine when a
                              leaf node must be split. It is the maximum number of
                              objects referenced by node before a split occurs.

     @param pi_rExtent - The extent of the node

     @param pi_rpParentNode - Reference to pointer to parent node

     @param IsUnsplitSubLevel - This parameter indicates to the created node wether
     it is an unsplit sub node. In this case, it knowns that its parent is not
     split and upon requiring a split, the parent should be requested to do it.
    -----------------------------------------------------------------------------*/
    HGFFeatureIndexNode<FEATURE, POINT, EXTENT> (size_t pi_SplitTreshold,
                                                 const EXTENT& pi_rExtent,
                                                 const HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >& pi_rpParentNode,
                                                 bool IsUnsplitSubLevel);

    /**----------------------------------------------------------------------------
     Copy constructor - This constructor may only be called when the
     given node has no parent

     @param pi_rNode - Reference to node to duplicate. This node must have no parent
    -----------------------------------------------------------------------------*/
    HGFFeatureIndexNode<FEATURE, POINT, EXTENT> (const HGFFeatureIndexNode<FEATURE, POINT, EXTENT>& pi_rNode);

    /**----------------------------------------------------------------------------
     Alternate Copy constructor ... equivalent but parent node is provided as parameter

     @param pi_rNode - Reference to node to duplicate.

     @param pi_rpParentNode - Pointer to parent node
    -----------------------------------------------------------------------------*/
    HGFFeatureIndexNode<FEATURE, POINT, EXTENT>(const HGFFeatureIndexNode& pi_rNode,
                                                const HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >& pi_rpParentNode);

    /**----------------------------------------------------------------------------
     Constructor for node loaded from file. Only the header is loaded from the file
     The actual features are kept on disk till needed.

     @param blockID The indentification of the feature on storage.

     @param parent The parent node.

     @param store the store the feature should be loaded from.

    -----------------------------------------------------------------------------*/
    HGFFeatureIndexNode(HPMBlockID blockID,
                        HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > parent,
                        HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> > store);

    /**----------------------------------------------------------------------------
     Destroyer
    -----------------------------------------------------------------------------*/
    virtual ~HGFFeatureIndexNode<FEATURE, POINT, EXTENT> ();

    /**----------------------------------------------------------------------------
     Clone
     These methods create a duplicate of the self node of the same type. The node
     has no parent and no children except for the Clonechild() which automatically
     sets the parent relationship. The CloneUnsplitChild creates a child node
     that is an unsplit additional level in the index
    -----------------------------------------------------------------------------*/
    virtual HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > Clone () const;
    virtual HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > Clone (const EXTENT& newNodeExtent) const;
    virtual HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > CloneChild (const EXTENT& newNodeExtent) const;
    virtual HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > CloneUnsplitChild (const EXTENT& newNodeExtent) const;


    /**----------------------------------------------------------------------------
      Get dirty flag
    -----------------------------------------------------------------------------*/
    bool IsDirty() const;

    /**----------------------------------------------------------------------------
      Set dirty flag
    -----------------------------------------------------------------------------*/
    virtual void SetDirty(bool dirty) const; // Intentionaly const ... only mutable members are modified

    /**----------------------------------------------------------------------------
     Returns the node blockID
    -----------------------------------------------------------------------------*/
    virtual HPMBlockID GetBlockID() const;

    /**----------------------------------------------------------------------------
     Returns the store
    -----------------------------------------------------------------------------*/
    virtual HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT>> GetStore() const;

    /**----------------------------------------------------------------------------
     Changes the store
    -----------------------------------------------------------------------------*/
    virtual bool ChangeStore(HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> > newStore);

    /**----------------------------------------------------------------------------
     Stores the present node on store (Discard) and stores all sub-nodes prior to this
    -----------------------------------------------------------------------------*/
    virtual bool Store();

    /**----------------------------------------------------------------------------
     Indicates if the node is currently discarded or not.
    -----------------------------------------------------------------------------*/
    virtual bool Discarded() const;

    /**----------------------------------------------------------------------------
     Gets a list of features potentially located inside given extent. The method does
     not perform precise geometric calculations to determine presence of feature
     in given extent. It simply compares the extents of spatial object and given extent
     to see if they overlap. This implies that some or many features may be located
     completely outside of the given extent.

     @param pi_rExtent The extent to obtain objects in the vicinity of.

     @param pio_rListOfObjects The list of objects to which are appended objects inside
                              the given extent

     @return The number of objects located.
    -----------------------------------------------------------------------------*/
    virtual size_t GetIn(const EXTENT& pi_rExtent,
                         list<FEATURE>& pio_rListOfObjects) const;


    /**----------------------------------------------------------------------------
     Loads the present tile if delay loaded.
    -----------------------------------------------------------------------------*/
    void Load() const; // Intentionaly const as only mutable members are modified

    /**----------------------------------------------------------------------------
     Unloads the present tile if delay loaded.
    -----------------------------------------------------------------------------*/
    void Unload() const; // Intentionaly const as only mutable members are modified

    virtual bool Destroy();


    /**----------------------------------------------------------------------------
     Returns the total number of points used by all features present in the node and
     its subnodes.
    -----------------------------------------------------------------------------*/
    virtual size_t
    GetPointCount () const;

    virtual void               ValidateInvariants() const
        {
        HGFIndexNode<FEATURE, POINT, EXTENT, vector<FEATURE>, HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >, HGFFeatureNodeHeader<EXTENT> >::ValidateInvariants();
        }

protected:

    virtual bool Discard() const;
    virtual bool Inflate() const;

    /**----------------------------------------------------------------------------
     Sets the store
     This method can only be called internally to set the store when it has not been
     set already.
    -----------------------------------------------------------------------------*/
    virtual bool SetStore(HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> > newStore);

    /**----------------------------------------------------------------------------
     Sets the sub nodes ... this can only be for a leaf node and the subnodes must
     exist.

     @param pi_apSubNodes - Array of four sub-nodes featureer that must point to existing
                           nodes
    -----------------------------------------------------------------------------*/
    void SetSubNodes(HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > pi_apSubNodes[], size_t numSubNodes);

//        bool              PreQuery (IHGFFeatureIndexQuery<POINT, EXTENT>* queryObject);
//        bool              Query (IHGFFeatureIndexQuery<POINT, EXTENT>* queryObject, list<POINT>& features);
//        bool              PostQuery (IHGFFeatureIndexQuery<POINT, EXTENT>* queryObject);



    /**----------------------------------------------------------------------------
     This method advises that one of the subnode storage block ID has changed
     as a result of reallocation, destruction or initial store.

     @param p_subNode - Pointer to sub node for which the ID changed
    -----------------------------------------------------------------------------*/
    virtual void AdviseSubNodeIDChanged(const HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >& p_subNode);


private:
    HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> > m_store;
    mutable HPMBlockID m_storeBlockID;
    mutable bool m_discarded;
    mutable bool m_dirty;

    mutable int32_t m_TotalPointsCount;


    };


















template<class FEATURE, class POINT, class EXTENT> class HGFFeatureIndex : public HGFSpatialIndex <FEATURE, POINT, EXTENT, HGFFeatureIndexNode<FEATURE, POINT, EXTENT>, HGFFeatureIndexHeader<EXTENT> >
    {

public:

    /**----------------------------------------------------------------------------------------------
     Constructor for this class. The split threshold is used to indicate the maximum
     amount of spatial objects to be indexed by an index node after which the node is
     split.

        @param store IN The store to which features are stored. This store is used in conjunction with
            pool management system.

        @param pi_SplitTreshold IN The maximum number of items per spatial index
                                   node after which the node may be split.

        @param balanced IN If true indicates that the index must remain balanced.

    -------------------------------------------------------------------------------------------------*/
    HGFFeatureIndex(HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> > store, size_t SplitTreshold, bool balanced);

    /**----------------------------------------------------------------------------
     Destructor
     If the index has unstored nodes then those will be stored.
    -----------------------------------------------------------------------------*/
    virtual             ~HGFFeatureIndex();

    /**----------------------------------------------------------------------------
     This method returns the store

     @return The store

    -----------------------------------------------------------------------------*/
    HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> >
    GetStore() const;
    /**----------------------------------------------------------------------------
     Changes the store
    -----------------------------------------------------------------------------*/
    virtual bool        ChangeStore(HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> > newStore);

    /**----------------------------------------------------------------------------
     Gets a list of features potentially located inside given extent. The method does
     not perform precise geometric calculations to determine presence of feature
     in given extent. It simply compares the extents of spatial object and given extent
     to see if they overlap. This implies that some or many features may be located
     completely outside of the given extent.

     @param pi_rExtent The extent to obtain objects in the vicinity of.

     @param pio_rListOfObjects The list of objects to which are appended objects inside
                              the given extent

     @return The number of objects located.

    -----------------------------------------------------------------------------*/
    size_t              GetIn(const EXTENT& pi_rExtent,
                              list<FEATURE>& pio_rListOfObjects) const;


    /**----------------------------------------------------------------------------
     Returns the total number of points used by all features present in the index
    -----------------------------------------------------------------------------*/
    virtual size_t
    GetPointCount () const;

//        size_t              Query (IHGFFeatureIndexQuery<POINT, EXTENT>* queryObject,
//                                   list<POINT>& resultFeatures);




protected:

    HGFFeatureIndex(const HGFFeatureIndex&   pi_rSpatialObject);

    HGFFeatureIndex&    operator=(const HGFFeatureIndex& pi_rObj);


    void                ValidateInvariants() const
        {
        };


private:
    HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> > m_store;
    };









/*======================================================================================================
** The IHGFFeatureIndexQuery provides a mechanism to implement a generic query for the feature index
**
** An object that implements this interface is provided to the feature index query function that will call
** as needed the query object, providing this object with nodes and subnodes iteratively through
** the index traversal.
**
** The interface defines many methods that will be called at different stages of the query process.
**
** The first one called is the GlobalPreQuery() method. This method is provided with the
** index upon which the query is to be performed and the target list of result features.
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
** nodes part of the index. It is at this stage and the current HGFFeatureIndexQuery implementation
** that features are gathered and copied into the result set. If the Query function returns false, then
** subnodes of the node being queired will not be queried, thus stopping the Query traversal for this branch
** of the index.
**
** Exactly the same way as the PreQuery, a PostQuery is called for every node of the index. The PostQuery
** can be used to clean any changes of states that may have been brought during the PreQuery. For example
** a table of feature oriented data may have been created during the Query process that now needs to be
** cleaned. Again returning false will stop the traversal process for the branch
**
** Finally a GlobalPostQuery method is called. This method can typically be used to analyse and modify
** the result feature set. It can also be used to put the index pre-query state if it has been changed.
**
** NOTE: It is not recommendable to modify any state of the nodes or the index during the process
** but for practical purposes, a limited amount of state can be modified by the query object.
** For this reason the query process will be executed as a single step upon tyhe index, forbidding during the
** process any other threads to operate upon the index or nodes. For this same reason, the PostQuery process
** and the GlobalPostQuery process will always complete regardless an error or exception is thrown.
**====================================================================================================*/

template<class FEATURE, class POINT, class EXTENT> class IHGFFeatureIndexQuery
    {
public:
    IHGFFeatureIndexQuery() {};
    virtual ~IHGFFeatureIndexQuery() {};

    /*======================================================================================================
    ** The GlobalPreQuery method is called before the pre-query and query process ... It provides a way
    ** for the query object to allocate ressources or do whatever is requesred prior to starting the
    ** process. Global preQuery is not usually required.
    ** Since most query objects do not need a gloabl pre-query performed a default implementation is provided.
    **
    ** @return true if the query must continue and false to cancle it all together. If false neither
    ** pre-query, query or post-query will be called, but the global post-query will still be callsed
    **====================================================================================================*/
    virtual bool        GlobalPreQuery (HGFFeatureIndex<FEATURE, POINT, EXTENT>& index,
                                        list<FEATURE>& features) {
        return true;
        };


    /*======================================================================================================
    ** The PreQuery method is called before the query process ... It provides a way for the query object
    ** to determine before the query proper starts, conditions to apply to the query processing.
    ** Since not all query objects need a pre-query performed a default implementation is provided.
    **
    ** @return true if the pre-query must continue and false otherwise. Returning false does to stop
    ** the query process, it only stops the pre-query of digging deeper into the feature index tree structure.
    **====================================================================================================*/
    virtual bool        PreQuery (HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > node,
                                  HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > subNodes[],
                                  size_t numSubNodes) {
        return false;
        };


    /*======================================================================================================
    ** This method gathers all features from the node that satisfy the query. The
    ** subnodes are provided for determination of the features that must be gathered only.
    ** The subnodes will be queried after this call UNLESS false is returned, thus
    ** indicating that query conditions have been reached for this branch of the index
    ** and digging down any further is unnecessary. The query process will not be altogether cancelled
    ** to completely cancel the query process, it is possible for all subsequent calls to this Query method
    ** to immediately return false. The hierarchical and recursive nature of the index insures that the query
    ** process will complete promptly in just a few calls.
    **
    ** Even if the query is stopped, the post-query process will be called
    **====================================================================================================*/
    virtual bool        Query (HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > node,
                               HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               list<FEATURE>& features) = 0;


    /*======================================================================================================
    ** The PostQuery method is called after the query process ... It provides a way for the query object
    ** to clean Resources filter or format result set.
    ** Since not all query objects need a post-query performed a default implementation is provided.
    **
    ** @return true if the post-query must continue and false otherwise. Even if false is returned the
    ** global post-query process will be called.
    **====================================================================================================*/
    virtual bool        PostQuery (HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > node,
                                   HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes) {
        return false;
        };

    /*======================================================================================================
    ** The GlobalPostQuery method is called after the query process and the Post-Query... It provides a way
    ** for the query object to allocate Resources or do whatever is requesred prior to starting the
    ** process. Global preQuery is not usually required.
    ** Since most query objects do not need a gloabl pre-query performed a default implementation is provided.
    **
    ** @return true if the query was successful and false otherwise. This value may be returned as indication
    ** of the global success of the query
    **====================================================================================================*/
    virtual bool        GlobalPostQuery (HGFFeatureIndex<FEATURE, POINT, EXTENT>& index,
                                         list<FEATURE>& features) {
        return true;
        };

    };

#include "HGFFeatureIndex.hpp"