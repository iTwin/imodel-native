/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/XYZRangeTree.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/
/*__PUBLISH_SECTION_START__*/
/*__PUBLISH_SECTION_END__*/

#include <algorithm>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

class XYZRangeTreeRoot;
class XYZRangeTreeLeaf;
class XYZRangeTreeInterior;
class XYZRangeTreeFunctions;
class XYZRangeTreeDump;
class XYZRangeTreeQuery;

typedef XYZRangeTreeInterior * XYZRangeTreeInteriorP;
typedef XYZRangeTreeQuery* XYZRangeTreeQueryP;
typedef XYZRangeTreeLeaf* XYZRangeTreeLeafP;
typedef XYZRangeTreeRoot* XYZRangeTreeRootP;
typedef void* XYZRangeTreeLeafDataType;
#define XYZRangeTreeLeafDataTypeNULL    (NULL)

/// <summary> Abstract method interface with query methods called during tree traversal.
///     Each method is called at particular stages of traversal.
///     The method can carry out application-specific computations for and return true/false
///         to guide subsequent traversal.
///  <summary>
class GEOMDLLIMPEXP XYZRangeTreeHandler
{
public:
/// <summary>Called on entry to an interior node.
///     Based on range of the subtree, the method can decide whether or not to recurse to children
/// </summary>
/// <return>true if children are to be examined.
///         false to continue immediately to ShouldContinueAfterLeaf
/// </return>
/// <param name="pRoot">Root node of entire tree</param>
/// <param name="pInterior">interior node to test</param>
GEOMAPI_VIRTUAL bool ShouldRecurseIntoSubtree (XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior)
    {
    return true;
    }

/// <summary>Called after completing processing of an interior node</summary>
/// <return>true if traversal should continue.
///         false to terminate entire search
///  </return>
/// <param name="pRoot">Root node of entire tree</param>
/// <param name="pInterior">interior node to test</param>
GEOMAPI_VIRTUAL bool ShouldContinueAfterSubtree      (XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior)
    {return true;}
/// <summary>Called for processing of a leaf node.</summary>
/// <return>true if traversal should continue.
///         false to terminate entire search
///  </return>
/// <param name="pRoot">Root node of entire tree</param>
/// <param name="pInterior">immediate interior node parent</param>
/// <param name="pLeaf">leaf node to test</param>
GEOMAPI_VIRTUAL bool ShouldContinueAfterLeaf         (XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior, XYZRangeTreeLeafP pLeaf)
    {return true;}
};

/// <summary>
///     Abstract method interface for root, interior, and leaf nodes within a range tree.
/// </summary>
class GEOMDLLIMPEXP XYZRangeTreeQuery
{
public:
GEOMAPI_VIRTUAL ~XYZRangeTreeQuery(){;}
// <summary>Return pointer to parent node in range tree.</summary>
GEOMAPI_VIRTUAL XYZRangeTreeQuery* Parent () = 0;
// <summary>Return xy range</summary>
GEOMAPI_VIRTUAL DRange3d Range () = 0;
// <summary>Fast cast to leaf node.  Return NULL from all other node classes.</summary>
GEOMAPI_VIRTUAL XYZRangeTreeLeaf* AsLeaf () = 0;
// <summary>Fast cast to interior node.  Return NULL from all other node classes.</summary>
GEOMAPI_VIRTUAL XYZRangeTreeInteriorP AsInterior () = 0;
// <summary>Fast cast to root node.  Return NULL from all other node classes.</summary>
GEOMAPI_VIRTUAL XYZRangeTreeRoot* AsRoot () = 0;

#define     DECLARE_OVERRIDES_XYZRangeTreeQueryNode \
    XYZRangeTreeQuery* Parent() override;     \
    DRange3d Range() override;                     \
    XYZRangeTreeLeafP     AsLeaf() override;        \
    XYZRangeTreeInteriorP AsInterior() override;    \
    XYZRangeTreeRootP     AsRoot() override;
};


/// ==============================================================================
/// <summary>Data node hanging from the tree.</summary>
/// ==============================================================================
class GEOMDLLIMPEXP XYZRangeTreeLeaf : public XYZRangeTreeQuery
{
private:
    DRange3d                mRange;
    XYZRangeTreeLeafDataType          mData;
    XYZRangeTreeQueryP mpParent;

    friend class XYZRangeTreeRoot;
    friend class XYZRangeTreeFunctions;
    friend class XYZRangeTreeDump;

// CONSTRUCTOR
XYZRangeTreeLeaf ();
public:
/// CONSTRUCTOR
XYZRangeTreeLeaf  (XYZRangeTreeLeafDataType data, DRange3d const&range);

/// <summary>Return user data from the leaf</summary>
XYZRangeTreeLeafDataType GetData ();
/// <summary>Set user data in the leaf</summary>
void SetData (XYZRangeTreeLeafDataType data);

DECLARE_OVERRIDES_XYZRangeTreeQueryNode
};


#define MAX_TREE_CHILD 50

struct XYZRangeTreeSplitCandidateArray;
/// ==============================================================================
/// <summary>Tree node that can have multiple tree nodes as descendants.</summary>
/// ==============================================================================
class GEOMDLLIMPEXP XYZRangeTreeInterior : public XYZRangeTreeQuery
{
    friend class XYZRangeTreeFunctions;
    friend struct XYZRangeTreeSplitCandidateArray;
    friend class XYZRangeTreeLeaf;
    friend class XYZRangeTreeDump;
    friend struct XYZRangeTreeMultiSearch;
private:

    DRange3d mRange;
    double   mExtentSquared;
    int mInstanceId;
    XYZRangeTreeQueryP mpParent;
    XYZRangeTreeQueryP mChild[MAX_TREE_CHILD];
    int mNumChild;
    bool mIsFringe;

    // Trim off children.  Parent pointer left untouched.
    void InitChildren ();
    // PRIVATE constructor
    XYZRangeTreeInterior ();

    // Append child to array.
    // NO  Range and extent are NOT updated.
    // YES mIsFringe IS updated
    // YES parent pointer in child is updated.
    /// <return>false if node is full</return>
    bool AddChild (XYZRangeTreeQueryP pChild);
    // <summary>test if node is full</summary>
    bool IsFull ();


public:
    DECLARE_OVERRIDES_XYZRangeTreeQueryNode

    /// <summary>return true if this node has LEAF nodes
    /// </summary>
    /// <remark>Update algorithms ensure that interior nodes
    ///     are not "mixed" -- children are either "all leaves"
    ///     or "all interior"
    /// </remark>
    bool IsFringe ();
    /// <summary>return squared size of node</summary>
    double ExtentSquared ();
    /// <summary>Set node range.  Updates extent</summary>
    void SetRange (DRange3d &range);
    /// <summary>Get child by index</summary>
    bool TryGetChild (int index, XYZRangeTreeQueryP &child);
    //
    bool Traverse (XYZRangeTreeRootP pRoot, XYZRangeTreeHandler &handler);
    void FreeChildren ();
};

/// <summary>Top level object for xy range tree</summary>
class GEOMDLLIMPEXP XYZRangeTreeRoot : public XYZRangeTreeQuery
{

private:
    friend class XYZRangeTreeFunctions;
    friend class XYZRangeTreeDump;
    friend struct XYZRangeTreeMultiSearch;

    XYZRangeTreeRoot (); // Private constructor.  Create tree with static Allocate
    XYZRangeTreeInteriorP mChild;
    void Traverse
            (
            XYZRangeTreeInteriorP pNode,
            XYZRangeTreeHandler &handler
            );

public:
    /// <summary>Allocate an empty XYZRangeTree</summary>
    static XYZRangeTreeRootP Allocate ();

    /// <summary>Free XYZRangeTree root and all contained nodes.</summary>
    static void Free (XYZRangeTreeRootP pRoot);

    /// <summary>Add user data with range</summary>
    bool Add (XYZRangeTreeLeafDataType data, DRange3d const&range);
    /// <summary>traverse the tree, making handler callbacks to announce search steps</summary>
    bool Traverse (XYZRangeTreeHandler &handler);

    DECLARE_OVERRIDES_XYZRangeTreeQueryNode
};



/// The following classes are examples of XYZRangeTreeHandler classes.
/// These are useful both for the services they provide and as examples
/// of how to code "handlers" for other application-specific searches.

///
/// <summary>Example "handler" class with traversal callbacks to accumulate statistics
///     about node types and depths.
/// </summary>
/// <usageSequence>
///     XYZRangeTreeCounter XYZRangeTreeCounter = XYZRangeTreeCounter ();
///     pTree->Traverse (searcher);
///     Results (counts) are found in various member variables after the traversal.
/// </usageSequence>
class GEOMDLLIMPEXP XYZRangeTreeCounter : public XYZRangeTreeHandler
{
public:
    int mNumLeaf;           // RESULT -- number of leaf objects
    int mNumFringe;         // RESULT -- number of interior nodes with leaf children
    int mNumInterior;       // RESULT -- number of interior nodes
    int mDepth;
    int mMaxDepth;            // RESULT -- maximum depth observed at any leaf
    double mMinFringeExtent;  // RESULT -- minimum size of any fringe node (interior node whose children are leaves)
    double mMaxFringeExtent;  // RESULT -- maximum size of any fringe node
    double mFringeExtentSum;  // RESULT -- sum of extents of all fringe nodes
    int mFringeLeafCount;   // INTERMEDIATE -- leavew within current node.
    int64_t mSumFringeLeafCountSquared; // Sum of squared leaf counts at fringe nodes.

/// <summary>Reinitialize for further searches.</summary>
void Init ()
    {
    mDepth = mNumLeaf = mNumInterior = mNumFringe = mMaxDepth = 0;
    mMinFringeExtent  =  DBL_MAX;
    mMaxFringeExtent  = -DBL_MAX;
    mFringeExtentSum = 0;
    mSumFringeLeafCountSquared = 0;
    mFringeLeafCount = 0;
    }

/// CONSTRUCTOR
XYZRangeTreeCounter (){Init();}

/// <summary>Record depth increase.</summary>
bool ShouldRecurseIntoSubtree       (XYZRangeTreeRootP    , XYZRangeTreeInteriorP    ) override 
        {
        mDepth++;
        mFringeLeafCount = 0;
        return true;
        }

/// <summary>
///     Record depth decrease.
///     Update fringe and interior statistics. Decrease depth.
/// </summary>
bool ShouldContinueAfterSubtree      (XYZRangeTreeRootP    , XYZRangeTreeInteriorP pInterior) override 
        {
        if (pInterior->IsFringe ())
            {
            mNumFringe++;
            double a = pInterior->ExtentSquared ();
            if (a  < mMinFringeExtent )
                mMinFringeExtent = a;
            if (a  > mMaxFringeExtent )
                mMaxFringeExtent = a;
            mFringeExtentSum += a;

            mSumFringeLeafCountSquared += mFringeLeafCount * mFringeLeafCount;
            }
        mNumInterior++;
        mDepth--;
        return true;
        }

/// <summary>Update leaf statistics.</summary>
bool ShouldContinueAfterLeaf         (XYZRangeTreeRootP    , XYZRangeTreeInteriorP    , XYZRangeTreeLeafP         ) override 
        {
        mNumLeaf++;
        mFringeLeafCount++;
        if (mDepth > mMaxDepth)
            mMaxDepth++;
        return true;
        }
};



/// <summary>Example "handler" class with traversal callbacks to search for leaf node whose low point
///     is closest to a target point.  The same XYZRangeTreeClosestLowPointSearcher object may be
///     used for multiple searches.
/// </summary>
/// <usageSequence>
///     XYZRangeTreeClosestLowPointSearcher searcher = XYZRangeTreeClosestLowPointSearcher ();
///     for each target point
///         {
///         DPoint3d targetPoint = ...
///         maxDist = .... // search for points within this distance of targetPoint.
///         searcher.Setup (targetPoint, maxDist);
///         pTree->Traverse (searcher);
///         Results available:
///            mClosestPoint is closest range low point
///            mMinDist2 is squared distance
///            mTag is data from closest point.
///         }
/// </usageSequence>
///
class GEOMDLLIMPEXP XYZRangeTreeClosestLowPointSearcher: public XYZRangeTreeHandler
{
public:
// CONSTRUCTOR
//
XYZRangeTreeClosestLowPointSearcher (){}

    DPoint3d mTarget;           // INPUT -- target point for search

    DRange3d mHitRange;         // SEARCH STATE -- range with target at center, xy extent to either
                                //     side is distance to closest point so far.
    double   mMinDist2;         // SEARCH STATE -- distance to closest point so far.

    DPoint3d mClosestPoint;     // RESULT -- closest point coordinates
    XYZRangeTreeLeafDataType mTag;        // RESULT -- data at closest point

    int      mInteriorTrue;     // Counts interior nodes whose children were examined.
    int      mInteriorFalse;    // Counts interior nodes whose children were NOT examined.
    int      mLeaf;             // Counts leaf nodes.
    int      mReduceDistance;   // Counts leaf nodes where distance-to-target was improved

// <summary>Call once per search point to initialize search state </summary>
void Setup (DPoint3d targetPoint, double maxDist)
    {
    mMinDist2 = DBL_MAX;
    mTarget.Zero ();
    mTarget = targetPoint;
    mHitRange.InitFrom (mTarget);
    mHitRange.Extend (maxDist);
    mTag = NULL;
    mInteriorTrue = mInteriorFalse = mLeaf = mReduceDistance = 0;
    }

// <summary>Return true if the current mHitRange overlaps the range of pNode.</summary>
bool ShouldRecurseIntoSubtree
(
XYZRangeTreeRootP    ,
XYZRangeTreeInteriorP pNode
) override 
    {
    DRange3d nodeRange = pNode->Range ();
    if (!nodeRange.IntersectsWith (nodeRange))
        {
        mInteriorFalse++;
        return false;
        }
    mInteriorTrue++;
    return true;
    }

// <summary>Always return true</summary>
bool ShouldContinueAfterSubtree
    (XYZRangeTreeRootP    ,XYZRangeTreeInteriorP pInterior) override {return true;}

/// <summary>Compute distance from target to pLeaf.
/// Update mHitRange and distance as needed</summary>
bool ShouldContinueAfterLeaf
    (XYZRangeTreeRootP    , XYZRangeTreeInteriorP    , XYZRangeTreeLeafP pLeaf) override 
    {
    mLeaf++;
    DRange3d range = pLeaf->Range ();
    double a = mTarget.DistanceSquared (range.low);
    if (a < mMinDist2)
        {
        mMinDist2 = a;
        double b = sqrt (a);
        mTag = pLeaf->GetData ();
        mReduceDistance++;
        mHitRange.InitFrom (mTarget);
        mHitRange.Extend (b);
        mClosestPoint = range.low;
        }
    return true;
    }
};

struct PolyfaceRangeTree;
typedef RefCountedPtr<PolyfaceRangeTree>  PolyfaceRangeTreePtr;

// Test queries for simultaneous recursion in two range trees.
struct DRange3dPairRecursionHandler
{
GEOMAPI_VIRTUAL bool TestInteriorInteriorPair (DRange3dCR rangeA, DRange3dCR rangeB) = 0;
GEOMAPI_VIRTUAL bool TestInteriorLeafPair (DRange3dCR rangeA, DRange3dCR rangeB, size_t indexB) = 0;
GEOMAPI_VIRTUAL void TestLeafLeafPair (DRange3dCR rangeA, size_t indexA, DRange3dCR rangeB, size_t indexB) = 0;
GEOMAPI_VIRTUAL bool TestLeafInteriorPair (DRange3dCR rangeA, size_t indexA, DRange3dCR rangeB) = 0;
GEOMAPI_VIRTUAL bool StillSearching () = 0;
};

/// <summary> Abstract method interface with query methods called during tree traversal.
///     Each method is called at particular stages of traversal.
///     The method can carry out application-specific computations for and return true/false
///         to guide subsequent traversal.
///  <summary>
struct DRange3dRecursionHandler
{
public:
/// <summary>Called on entry to a subtree.
/// </summary>
/// <return>true if children are to be examined.
/// </return>
/// <param name="range">range containing all members of the node.</param>
GEOMAPI_VIRTUAL bool ShouldRecurseIntoSubtree (DRange3dCR range)
    {
    return true;
    }

/// <summary> called to ask if the search should continue.</summary>
GEOMAPI_VIRTUAL bool IsActive ()
    {
    return true;
    }
    
/// <summary> called to announce visit to a leaf.</summary>
GEOMAPI_VIRTUAL void AnnounceLeaf (DRange3dCR range, size_t index)
    {
    }    
};


struct XYZRangeTreeRecursionState
  {
  XYZRangeTreeInteriorP m_left;
  XYZRangeTreeInteriorP m_right;
  int m_leftIndex, m_rightIndex;

  XYZRangeTreeRecursionState (XYZRangeTreeInteriorP left, int leftIndex, XYZRangeTreeInteriorP right, int rightIndex);
  };

struct XYZRangeTreeMultiSearch;
typedef XYZRangeTreeMultiSearch &XYZRangeTreeMutltiSearchR;

// Range-based search support for polyface.
struct PolyfaceRangeTree : public RefCountedBase
{
private:
XYZRangeTreeRootP m_rangeTree;
PolyfaceRangeTree ();
~PolyfaceRangeTree ();
void ReleaseMem (bool allocateEmptyTree);

size_t LoadPolyface (PolyfaceQueryCR source);
public:
// Allocate a new searcher.
GEOMDLLIMPEXP static PolyfaceRangeTreePtr CreateForPolyface (PolyfaceQueryCR source);

// Find all facets in specified search range (with optional expansion)
GEOMDLLIMPEXP void CollectInRange (bvector<size_t> &hits, DRange3dCR range, double expansion = 0);

// get the range tree pointer.
GEOMDLLIMPEXP XYZRangeTreeRootP GetXYZRangeTree ();

// Search for clashing pairs.
//
static GEOMDLLIMPEXP void CollectClashPairs (
PolyfaceQueryR polyfaceA,           //!< first polyface
PolyfaceRangeTree &treeA,           //!< range tree for polyfaceA
PolyfaceQueryR polyfaceB,           //!< second polyface
PolyfaceRangeTree &treeB,           //!< range tree for polyfaceB
double proximity,                   //!< collect hits within this proximity.
bvector<std::pair<size_t, size_t>> &hits,   //!< read indices of clashing pairs
size_t maxHits,                      //! maximum number of hits to collect.
XYZRangeTreeMutltiSearchR searcher    //! (to be reused over multiple calls)
);
};

// Algorithm code for doing multiple searches of polyface range trees.
// Instantiate the searcher once.
// Reuse it for each (tree,tree) pair to be searched.  (This reuses the stack memory used for the search recursion)
struct XYZRangeTreeMultiSearch
{
private:

bvector <XYZRangeTreeRecursionState> m_stack;
void Push (XYZRangeTreeInteriorP left, int leftIndex, XYZRangeTreeInteriorP right, int rightIndex);

public:
// Find all facets in specified search range 
GEOMDLLIMPEXP void RunSearch (XYZRangeTreeRootP treeA, XYZRangeTreeRootP treeB, DRange3dPairRecursionHandler &tester);
};


// Sample range-box-only clash tester.
// User can further override TestLeafLeafPair to do more detailed test.
struct SimpleRangeClashTester : DRange3dPairRecursionHandler
{
size_t m_numHits;
double m_distance;
SimpleRangeClashTester (double envelope)
    : m_numHits (0),
      m_distance (envelope)
    {
    }

bool TestOverlap (DRange3dCR rangeA, DRange3dCR rangeB)
    {
    return rangeA.IntersectsWith (rangeB, m_distance, 3);
    }
bool TestInteriorInteriorPair (DRange3dCR rangeA, DRange3dCR rangeB) override  {return TestOverlap (rangeA, rangeB);}
bool TestInteriorLeafPair (DRange3dCR rangeA, DRange3dCR rangeB, size_t indexB) override  {return TestOverlap (rangeA, rangeB);}
void TestLeafLeafPair (DRange3dCR rangeA, size_t indexA, DRange3dCR rangeB, size_t indexB) override
    {
    if (rangeA.IntersectsWith (rangeB, m_distance, 3))
        m_numHits++;
    }
bool TestLeafInteriorPair (DRange3dCR rangeA, size_t indexA, DRange3dCR rangeB) override  {return TestOverlap (rangeA, rangeB);}

// Return false as soon as m_numHits is non-zero
bool StillSearching () override
    {
    return m_numHits == 0;
    }
};



/*__PUBLISH_SECTION_START__*/

struct IRangeTree3d;
typedef RefCountedPtr<IRangeTree3d>  IRangeTree3dPtr;

//! Public/Protected wrapper for range tree search implementations.
//!
struct IRangeTree3d : public RefCountedBase
{
protected:
IRangeTree3d ();
/*__PUBLISH_SECTION_END__*/

protected: 
GEOMAPI_VIRTUAL void _Traverse (DRange3dRecursionHandler &handler) = 0;

GEOMAPI_VIRTUAL bool _Add (DRange3dCR userRange, size_t userIndex) = 0;
/*__PUBLISH_SECTION_START__*/

public:

GEOMDLLIMPEXP bool Add (DRange3dCR userRange, size_t userIndex);

GEOMDLLIMPEXP void Traverse (DRange3dRecursionHandler &handler);

static GEOMDLLIMPEXP IRangeTree3dPtr Create ();
};


struct PolyfaceRangeTree01;
typedef RefCountedPtr<PolyfaceRangeTree01>  PolyfaceRangeTree01Ptr;

// Range-based search support for polyface.
struct PolyfaceRangeTree01 : public RefCountedBase
{
private:
IRangeTree3dPtr m_rangeTree;
PolyfaceRangeTree01 ();
~PolyfaceRangeTree01 ();

size_t LoadPolyface (PolyfaceQueryCR source);
public:
// Allocate a new searcher.
GEOMDLLIMPEXP static PolyfaceRangeTree01Ptr CreateForPolyface (PolyfaceQueryCR source);

// Find all facets in specified search range (with optional expansion)
GEOMDLLIMPEXP void CollectInRange (bvector<size_t> &hits, DRange3dCR range, double expansion = 0);

// Search for clashing pairs.
//
static GEOMDLLIMPEXP void CollectClashPairs (
PolyfaceQueryR polyfaceA,           //!< first polyface
PolyfaceRangeTree01 &treeA,           //!< range tree for polyfaceA
PolyfaceQueryR polyfaceB,           //!< second polyface
PolyfaceRangeTree01 &treeB,           //!< range tree for polyfaceB
bvector<std::pair<size_t, size_t>> &hits,   //!< read indices of clashing pairs
size_t maxHits                      //! maximum number of hits to collect.
);
};

END_BENTLEY_GEOMETRY_NAMESPACE
