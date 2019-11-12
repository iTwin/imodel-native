/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/
/*__PUBLISH_SECTION_START__*/
/*__PUBLISH_SECTION_END__*/

#include <algorithm>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

class XYRangeTreeRoot;
class XYRangeTreeLeaf;
class XYRangeTreeInterior;
class XYRangeTreeFunctions;
class XYRangeTreeDump;
class XYRangeTreeQuery;

typedef XYRangeTreeInterior * XYRangeTreeInteriorP;
typedef XYRangeTreeQuery* XYRangeTreeQueryP;
typedef XYRangeTreeLeaf* XYRangeTreeLeafP;
typedef XYRangeTreeRoot* XYRangeTreeRootP;
typedef void* XYRangeTreeLeafDataType;
#define XYRangeTreeLeafDataTypeNULL    (NULL)

/// <summary> Abstract method interface with query methods called during tree traversal.
///     Each method is called at particular stages of traversal.
///     The method can carry out application-specific computations for and return true/false
///         to guide subsequent traversal.
///  <summary>
class GEOMDLLIMPEXP XYRangeTreeHandler
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
GEOMAPI_VIRTUAL bool ShouldRecurseIntoSubtree (XYRangeTreeRootP pRoot, XYRangeTreeInteriorP pInterior)
    {
    return true;
    }

/// <summary>Called after completing processing of an interior node</summary>
/// <return>true if traversal should continue.
///         false to terminate entire search
///  </return>
/// <param name="pRoot">Root node of entire tree</param>
/// <param name="pInterior">interior node to test</param>
GEOMAPI_VIRTUAL bool ShouldContinueAfterSubtree      (XYRangeTreeRootP pRoot, XYRangeTreeInteriorP pInterior)
    {return true;}
/// <summary>Called for processing of a leaf node.</summary>
/// <return>true if traversal should continue.
///         false to terminate entire search
///  </return>
/// <param name="pRoot">Root node of entire tree</param>
/// <param name="pInterior">immediate interior node parent</param>
/// <param name="pLeaf">leaf node to test</param>
GEOMAPI_VIRTUAL bool ShouldContinueAfterLeaf         (XYRangeTreeRootP pRoot, XYRangeTreeInteriorP pInterior, XYRangeTreeLeafP pLeaf)
    {return true;}
};

/// <summary>
///     Abstract method interface for root, interior, and leaf nodes within a range tree.
/// </summary>
class GEOMDLLIMPEXP XYRangeTreeQuery
{
public:
// <summary>Does nothing; supports inheritance.</summary>
GEOMAPI_VIRTUAL ~XYRangeTreeQuery(){}
// <summary>Return pointer to parent node in range tree.</summary>
GEOMAPI_VIRTUAL XYRangeTreeQuery* Parent () = 0;
// <summary>Return xy range</summary>
GEOMAPI_VIRTUAL DRange2d Range () = 0;
// <summary>Fast cast to leaf node.  Return NULL from all other node classes.</summary>
GEOMAPI_VIRTUAL XYRangeTreeLeaf* AsLeaf () = 0;
// <summary>Fast cast to interior node.  Return NULL from all other node classes.</summary>
GEOMAPI_VIRTUAL XYRangeTreeInteriorP AsInterior () = 0;
// <summary>Fast cast to root node.  Return NULL from all other node classes.</summary>
GEOMAPI_VIRTUAL XYRangeTreeRoot* AsRoot () = 0;

#define     DECLARE_OVERRIDES_XYRangeTreeQueryNode \
    XYRangeTreeQuery* Parent() override;     \
    DRange2d Range() override;                     \
    XYRangeTreeLeafP     AsLeaf() override;        \
    XYRangeTreeInteriorP AsInterior() override;    \
    XYRangeTreeRootP     AsRoot() override;
};


/// ==============================================================================
/// <summary>Data node hanging from the tree.</summary>
/// ==============================================================================
class GEOMDLLIMPEXP XYRangeTreeLeaf : public XYRangeTreeQuery
{
private:
    DRange2d                mRange;
    XYRangeTreeLeafDataType          mData;
    XYRangeTreeQueryP mpParent;

    friend class XYRangeTreeRoot;
    friend class XYRangeTreeFunctions;
    friend class XYRangeTreeDump;

// CONSTRUCTOR
XYRangeTreeLeaf ();
public:
/// CONSTRUCTOR
XYRangeTreeLeaf  (XYRangeTreeLeafDataType data, DRange2d const&range);

/// <summary>Return user data from the leaf</summary>
XYRangeTreeLeafDataType GetData ();
/// <summary>Set user data in the leaf</summary>
void SetData (XYRangeTreeLeafDataType data);

DECLARE_OVERRIDES_XYRangeTreeQueryNode
};


#define MAX_TREE_CHILD 50

struct XYRangeTreeSplitCandidateArray;
/// ==============================================================================
/// <summary>Tree node that can have multiple tree nodes as descendants.</summary>
/// ==============================================================================
class GEOMDLLIMPEXP XYRangeTreeInterior : public XYRangeTreeQuery
{
    friend class XYRangeTreeFunctions;
    friend struct XYRangeTreeSplitCandidateArray;
    friend class XYRangeTreeLeaf;
    friend class XYRangeTreeDump;

private:

    DRange2d mRange;
    double   mExtentSquared;
    int mInstanceId;
    XYRangeTreeQueryP mpParent;
    XYRangeTreeQueryP mChild[MAX_TREE_CHILD];
    int mNumChild;
    bool mIsFringe;

    // Trim off children.  Parent pointer left untouched.
    void InitChildren ();
    // PRIVATE constructor
    XYRangeTreeInterior ();

    // Append child to array.
    // NO  Range and extent are NOT updated.
    // YES mIsFringe IS updated
    // YES parent pointer in child is updated.
    /// <return>false if node is full</return>
    bool AddChild (XYRangeTreeQueryP pChild);
    // <summary>test if node is full</summary>
    bool IsFull ();


public:
    DECLARE_OVERRIDES_XYRangeTreeQueryNode

    GEOMAPI_VIRTUAL ~XYRangeTreeInterior() {;}

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
    void SetRange (DRange2d &range);

    bool TryGetChild (int index, XYRangeTreeQueryP &child);
    //
    bool Traverse (XYRangeTreeRootP pRoot, XYRangeTreeHandler &handler);
    void FreeChildren ();
};


/// <summary>Top level object for xy range tree</summary>
class GEOMDLLIMPEXP XYRangeTreeRoot : public XYRangeTreeQuery
{

private:
    friend class XYRangeTreeFunctions;
    friend class XYRangeTreeDump;

    DRange2d mRange;
    XYRangeTreeRoot (); // Private constructor.  Create tree with static Allocate
    XYRangeTreeInteriorP mChild;
    void Traverse
            (
            XYRangeTreeInteriorP pNode,
            XYRangeTreeHandler &handler
            );

public:
    /// <summary>Allocate an empty XYRangeTree</summary>
    static XYRangeTreeRootP Allocate ();

    GEOMAPI_VIRTUAL ~XYRangeTreeRoot() {;}

    /// <summary>Free XYRangeTree root and all contained nodes.</summary>
    static void Free (XYRangeTreeRootP pRoot);

    /// <summary>Add user data with range</summary>
    bool Add (XYRangeTreeLeafDataType data, DRange2d const&range);
    /// <summary>traverse the tree, making handler callbacks to announce search steps</summary>
    bool Traverse (XYRangeTreeHandler &handler);

    DECLARE_OVERRIDES_XYRangeTreeQueryNode
};



/// The following classes are examples of XYRangeTreeHandler classes.
/// These are useful both for the services they provide and as examples
/// of how to code "handlers" for other application-specific searches.

///
/// <summary>Example "handler" class with traversal callbacks to accumulate statistics
///     about node types and depths.
/// </summary>
/// <usageSequence>
///     XYRangeTreeCounter XYRangeTreeCounter = XYRangeTreeCounter ();
///     pTree->Traverse (searcher);
///     Results (counts) are found in various member variables after the traversal.
/// </usageSequence>
class XYRangeTreeCounter : public XYRangeTreeHandler
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

/// <summary>Reinitialize for further searches.</summary>
void Init ()
    {
    mDepth = mNumLeaf = mNumInterior = mNumFringe = mMaxDepth = 0;
    mMinFringeExtent  =  DBL_MAX;
    mMaxFringeExtent  = -DBL_MAX;
    mFringeExtentSum = 0;
    }

/// CONSTRUCTOR
XYRangeTreeCounter (){Init();}

/// <summary>Record depth increase.</summary>
bool ShouldRecurseIntoSubtree       (XYRangeTreeRootP    , XYRangeTreeInteriorP    ) override 
        {
        mDepth++;
        return true;
        }

/// <summary>
///     Record depth decrease.
///     Update fringe and interior statistics. Decrease depth.
/// </summary>
bool ShouldContinueAfterSubtree      (XYRangeTreeRootP    , XYRangeTreeInteriorP pInterior) override 
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
            }
        mNumInterior++;
        mDepth--;
        return true;
        }

/// <summary>Update leaf statistics.</summary>
bool ShouldContinueAfterLeaf         (XYRangeTreeRootP    , XYRangeTreeInteriorP    , XYRangeTreeLeafP         ) override 
        {
        mNumLeaf++;
        if (mDepth > mMaxDepth)
            mMaxDepth++;
        return true;
        }
};



/// <summary>Example "handler" class with traversal callbacks to search for leaf node whose low point
///     is closest to a target point.  The same XYRangeTreeClosestLowPointSearcher object may be
///     used for multiple searches.
/// </summary>
/// <usageSequence>
///     XYRangeTreeClosestLowPointSearcher searcher = XYRangeTreeClosestLowPointSearcher ();
///     for each target point
///         {
///         DPoint2d targetPoint = ...
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
class GEOMDLLIMPEXP XYRangeTreeClosestLowPointSearcher: public XYRangeTreeHandler
{
public:
// CONSTRUCTOR
//
XYRangeTreeClosestLowPointSearcher (){}

    DPoint2d mTarget;           // INPUT -- target point for search

    DRange2d mHitRange;         // SEARCH STATE -- range with target at center, xy extent to either
                                //     side is distance to closest point so far.
    double   mMinDist2;         // SEARCH STATE -- distance to closest point so far.

    DPoint2d mClosestPoint;     // RESULT -- closest point coordinates
    XYRangeTreeLeafDataType mTag;        // RESULT -- data at closest point

    int      mInteriorTrue;     // Counts interior nodes whose children were examined.
    int      mInteriorFalse;    // Counts interior nodes whose children were NOT examined.
    int      mLeaf;             // Counts leaf nodes.
    int      mReduceDistance;   // Counts leaf nodes where distance-to-target was improved

// <summary>Call once per search point to initialize search state </summary>
void Setup (DPoint2d targetPoint, double maxDist)
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
XYRangeTreeRootP    ,
XYRangeTreeInteriorP pNode
) override 
    {
    DRange2d nodeRange = pNode->Range ();
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
    (XYRangeTreeRootP    ,XYRangeTreeInteriorP pInterior) override {return true;}

/// <summary>Compute distance from target to pLeaf.
/// Update mHitRange and distance as needed</summary>
bool ShouldContinueAfterLeaf
    (XYRangeTreeRootP    , XYRangeTreeInteriorP    , XYRangeTreeLeafP pLeaf) override 
    {
    mLeaf++;
    DRange2d range = pLeaf->Range ();
    double a = mTarget.DistanceSquared (*(&range.low));
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


END_BENTLEY_GEOMETRY_NAMESPACE