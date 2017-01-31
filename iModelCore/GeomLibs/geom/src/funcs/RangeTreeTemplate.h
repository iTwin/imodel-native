/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/RangeTreeTemplate.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


// Use #include to pull turn this file into source.
// Predefine RANGETREE_IS3D for 3d tree.  Otherse (i.e. RANGETREE_IS3D undefined) tree is 2d.

#ifdef RANGETREE_IS3D

#define POINTTYPE DPoint3d
#define RANGETYPE DRange3d
#define RANGETREETYPE XYZRangeTree

#define __RangeTreeSplitCandidateArray__ XYZRangeTreeSplitCandidateArray
#define __RangeTreeLeaf__           XYZRangeTreeLeaf
#define __RangeTreeLeafData__       XYZRangeTreeLeafDataType

#define __RangeTreeLeafP__          XYZRangeTreeLeafP
#define __RangeTreeInteriorP__      XYZRangeTreeInteriorP
#define __RangeTreeFunctions__      XYZRangeTreeFunctions
#define __RangeTreeQueryP__         XYZRangeTreeQueryP
#define __RangeTreeRootP__          XYZRangeTreeRootP

#define __RangeTreeInterior__       XYZRangeTreeInterior
#define __RangeTreeRoot__           XYZRangeTreeRoot

#define __RangeTreeLeafDataNULL__   XYZRangeTreeLeafDataTypeNULL
#define __RangeTreeHandler__        XYZRangeTreeHandler

#else

#define POINTTYPE DPoint2d
#define RANGETYPE DRange2d
#define RANGETREETYPE XYRangeTree

#define __RangeTreeSplitCandidateArray__ XYRangeTreeSplitCandidateArray
#define __RangeTreeLeaf__           XYRangeTreeLeaf
#define __RangeTreeLeafData__       XYRangeTreeLeafDataType

#define __RangeTreeLeafP__          XYRangeTreeLeafP
#define __RangeTreeInteriorP__      XYRangeTreeInteriorP
#define __RangeTreeFunctions__      XYRangeTreeFunctions
#define __RangeTreeQueryP__         XYRangeTreeQueryP
#define __RangeTreeRootP__          XYRangeTreeRootP

#define __RangeTreeInterior__       XYRangeTreeInterior
#define __RangeTreeRoot__           XYRangeTreeRoot

#define __RangeTreeLeafDataNULL__   XYRangeTreeLeafDataTypeNULL
#define __RangeTreeHandler__        XYRangeTreeHandler

#endif

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct __RangeTreeSplitCandidateArray__;


static bool Extend (RANGETYPE &rangeA, RANGETYPE const &rangeB)
    {
    bool bExtended = false;

    if (rangeB.low.x < rangeA.low.x)
        {
        rangeA.low.x = rangeB.low.x;
        bExtended = true;
        }

    if (rangeB.high.x > rangeA.high.x)
        {
        rangeA.high.x = rangeB.high.x;
        bExtended = true;
        }

    if (rangeB.low.y < rangeA.low.y)
        {
        rangeA.low.y = rangeB.low.y;
        bExtended = true;
        }

    if (rangeB.high.y > rangeA.high.y)
        {
        rangeA.high.y = rangeB.high.y;
        bExtended = true;
        }

#ifdef RANGETREE_IS3D
    if (rangeB.low.z < rangeA.low.z)
        {
        rangeA.low.z = rangeB.low.z;
        bExtended = true;
        }

    if (rangeB.high.z > rangeA.high.z)
        {
        rangeA.high.z = rangeB.high.z;
        bExtended = true;
        }
#endif

    return bExtended;
    }

static void Init (RANGETYPE &range)
    {
    range.low.x  = range.low.y  =  DBL_MAX;
    range.high.x = range.high.y = -DBL_MAX;
#ifdef RANGETREE_IS3D
    range.low.z  = DBL_MAX;
    range.high.z = -DBL_MAX;
#endif
    }


/// ==============================================================================
/// <summary>Data node hanging from the tree.</summary>
/// ==============================================================================
__RangeTreeLeaf__::__RangeTreeLeaf__ ()
    {
    Init (mRange);
    mData = __RangeTreeLeafDataNULL__;
    mpParent = NULL;
    }


__RangeTreeLeaf__::__RangeTreeLeaf__  (__RangeTreeLeafData__ data, RANGETYPE const&range)
    {
    mRange = range;
    mpParent = NULL;
    mData = data;
    }

__RangeTreeQueryP__ __RangeTreeLeaf__::Parent ()   {return mpParent;}
RANGETYPE __RangeTreeLeaf__::Range ()                   {return mRange;}
__RangeTreeLeafP__ __RangeTreeLeaf__::AsLeaf ()             {return this;}
__RangeTreeInteriorP__ __RangeTreeLeaf__::AsInterior ()     {return NULL;}
__RangeTreeRootP__ __RangeTreeLeaf__::AsRoot ()             {return NULL;}

__RangeTreeLeafData__ __RangeTreeLeaf__::GetData ()           {return mData;}
void __RangeTreeLeaf__::SetData (__RangeTreeLeafData__ data)  {mData = data;}


static int sInteriorInstanceCounter;

// Trim off children.  Parent pointer left untouched.
void __RangeTreeInterior__::InitChildren ()
    {
    Init (mRange);
    mExtentSquared = DBL_MAX;
    mIsFringe = true;
    mNumChild = 0;
    }

// PRIVATE constructor
__RangeTreeInterior__::__RangeTreeInterior__ ()
    {
    InitChildren ();
    mpParent = NULL;
    mInstanceId = sInteriorInstanceCounter++;
    }

bool __RangeTreeInterior__::AddChild (__RangeTreeQueryP__ pChild)
    {
    if (mNumChild >= MAX_TREE_CHILD)
        return false;
    mChild[mNumChild++] = pChild;
    if (NULL == pChild->AsLeaf ())
        mIsFringe = false;
    __RangeTreeInteriorP__ pChildAsInterior = pChild->AsInterior ();
    if (NULL != pChildAsInterior)
        pChildAsInterior->mpParent = this;
    return true;
    }

bool __RangeTreeInterior__::IsFull ()  {return mNumChild >= MAX_TREE_CHILD;}


__RangeTreeQueryP__ __RangeTreeInterior__::Parent ()   {return mpParent;}
RANGETYPE __RangeTreeInterior__::Range ()                   {return mRange;}
__RangeTreeLeafP__ __RangeTreeInterior__::AsLeaf ()          {return 0;}
__RangeTreeInteriorP__ __RangeTreeInterior__::AsInterior ()     {return this;}
__RangeTreeRootP__ __RangeTreeInterior__::AsRoot ()          {return 0;}

bool __RangeTreeInterior__::IsFringe ()            {return mIsFringe;}
double __RangeTreeInterior__::ExtentSquared ()     {return mExtentSquared;}

void __RangeTreeInterior__::SetRange (RANGETYPE &range)
    {
    mRange = range;
    mExtentSquared = range.ExtentSquared ();
    }
bool __RangeTreeInterior__::TryGetChild (int index, __RangeTreeQueryP__ &child)
    {
    if (index >= 0 && index < mNumChild)
        {
        child = mChild[index];
        return true;
        }
    child = NULL;
    return false;
    }

bool __RangeTreeInterior__::Traverse (__RangeTreeRootP__ pRoot, __RangeTreeHandler__ &handler)
    {
    if (handler.ShouldRecurseIntoSubtree (pRoot, this))
        {
        for (int i = 0; i < mNumChild; i++)
            {
            __RangeTreeLeafP__ pLeaf = mChild[i]->AsLeaf ();
            if (NULL != pLeaf)
                {
                if (!handler.ShouldContinueAfterLeaf (pRoot, this, pLeaf))
                    return false;
                }
            else
                {
                __RangeTreeInteriorP__ pInterior = mChild[i]->AsInterior ();
                if (NULL != pInterior)
                    {
                    if (!pInterior->Traverse (pRoot, handler))
                        return false;
                    }
                }
            }
        }
    return handler.ShouldContinueAfterSubtree (pRoot, this);
    }

void __RangeTreeInterior__::FreeChildren ()
    {
    for (int i = 0; i < mNumChild; i++)
        {
        __RangeTreeInteriorP__ pInterior = mChild[i]->AsInterior ();
        if (NULL != pInterior)
            pInterior->FreeChildren ();
        delete mChild[i];
        mChild[i] = 0;
        }
    mNumChild = 0;
    }


__RangeTreeQueryP__     __RangeTreeRoot__::Parent ()     {return NULL;}
RANGETYPE               __RangeTreeRoot__::Range ()      {return NULL != mChild ? mChild->Range() : RANGETYPE::NullRange();}
__RangeTreeLeafP__      __RangeTreeRoot__::AsLeaf ()     {return NULL;}
__RangeTreeInteriorP__  __RangeTreeRoot__::AsInterior () {return NULL;}
__RangeTreeRootP__      __RangeTreeRoot__::AsRoot ()     {return this;}


bool __RangeTreeRoot__::Traverse (__RangeTreeHandler__ &handler)
    {
    if (NULL != mChild)
        return mChild->Traverse (this, handler);
    return true;
    }


void __RangeTreeRoot__::Free (__RangeTreeRootP__ pRoot)
    {
    if (NULL != pRoot->mChild)
        {
        pRoot->mChild->FreeChildren ();
        delete pRoot->mChild;
        pRoot->mChild = NULL;
        delete pRoot;
        }
    }


__RangeTreeRoot__::__RangeTreeRoot__ ()
    {
    mChild = NULL;
    }


__RangeTreeRootP__ __RangeTreeRoot__::Allocate ()
    {
    return new __RangeTreeRoot__ ();
    }


// Algorithmic part of range tree operations.
//
class __RangeTreeFunctions__
{
public:
    static __RangeTreeInteriorP__ AllocateInterior () {return new __RangeTreeInterior__ ();}
    static __RangeTreeLeafP__ AllocateLeaf () {return new __RangeTreeLeaf__ ();}

private:

friend class __RangeTreeRoot__;

static void CombineRanges (RANGETYPE &result, RANGETYPE const&rangeA, RANGETYPE const&rangeB)
    {
    result.low.x = rangeA.low.x < rangeB.low.x ? rangeA.low.x : rangeB.low.x;
    result.low.y = rangeA.low.y < rangeB.low.y ? rangeA.low.y : rangeB.low.y;
    result.high.x = rangeA.high.x > rangeB.high.x ? rangeA.high.x : rangeB.high.x;
    result.high.y = rangeA.high.y > rangeB.high.y ? rangeA.high.y : rangeB.high.y;
#ifdef RANGETREE_IS3D
    result.low.z = rangeA.low.z < rangeB.low.z ? rangeA.low.z : rangeB.low.z;
    result.high.z = rangeA.high.z > rangeB.high.z ? rangeA.high.z : rangeB.high.z;
#endif
    }

static double FitFunc (RANGETYPE const&newRange, __RangeTreeQueryP__ pNode)
    {
    if (NULL == pNode)
        return DBL_MAX;

    __RangeTreeInteriorP__ pInterior = pNode->AsInterior ();
    if (NULL == pInterior)
        return DBL_MAX;
    double      newExtent, increment;
    RANGETYPE    currRange = pInterior->Range ();
    RANGETYPE    expandedRange;
    CombineRanges (expandedRange, currRange, newRange);
    newExtent = expandedRange.ExtentSquared ();

    increment = newExtent - pInterior->mExtentSquared;

    /* "thisFit" is a somewhat arbitrary measure of how well the range fits into this
       node, taking into account the total size ("new extent") of this node plus this range,
       plus a penalty for increasing it from its existing size. */
    return newExtent + increment;
    }

static __RangeTreeInteriorP__ FindOrAddParentFor (RANGETYPE const&range, __RangeTreeRootP__ pRoot, __RangeTreeQueryP__ pSubtree)
    {
    if (NULL == pSubtree)
        {
        if (NULL != pRoot->mChild)
            return FindOrAddParentFor (range, pRoot, pRoot->mChild);
        // First addition to empty tree.
        __RangeTreeInteriorP__ pChild = AllocateInterior ();
        pRoot->mChild = pChild;
        pChild->mpParent = pRoot;
        return pChild;
        }

    __RangeTreeInteriorP__ pInterior = pSubtree->AsInterior ();
    if (NULL == pInterior)
        return NULL;

    if (pInterior->IsFringe ())
        return pInterior;

    double              thisFit, bestFit = DBL_MAX;
    __RangeTreeInteriorP__ pBest = NULL;
    for (int i = 0; i < pInterior->mNumChild; i++)
        {
        __RangeTreeInteriorP__ pChild = pInterior->mChild[i]->AsInterior ();
        thisFit = FitFunc (range, pChild);
        if (NULL == pBest || thisFit < bestFit)
            {
            pBest = pChild;
            bestFit = thisFit;
            }
        }
    return FindOrAddParentFor (range, pRoot, pBest);
    }

static bool RangeChangesAsExtended (__RangeTreeQueryP__ pNode, RANGETYPE &range)
    {
    if (NULL == pNode)
        return false;
    __RangeTreeInteriorP__ pInterior = pNode->AsInterior ();
    if (NULL == pInterior)
        return false;
    RANGETYPE nodeRange = pInterior->Range ();
    bool bChanged = Extend (nodeRange, range);
    if (bChanged)
        pInterior->SetRange (nodeRange);
    return bChanged;
    }



static void Split (__RangeTreeInteriorP__ pParent);

/// <param name="bFixupRanges">If true, redursively update ranges of ancestors.</param>
static bool InsertChild (__RangeTreeQueryP__ pQueryParent, __RangeTreeQueryP__ pChild, bool bFixupRanges)
    {
    if (NULL == pQueryParent)
        return false;

    __RangeTreeInteriorP__ pParent = pQueryParent->AsInterior ();

    if (NULL == pParent)
        {
        __RangeTreeRootP__ pRoot = pQueryParent->AsRoot ();
        __RangeTreeInteriorP__ pChildAsInterior = pChild->AsInterior ();
        if (NULL != pRoot)
            {
            __RangeTreeInteriorP__ pOldInterior = pRoot->mChild;
            if (NULL == pOldInterior && NULL != pChildAsInterior)
                {
                // Lonely parent, just insert.
                pRoot->mChild = pChildAsInterior;
                pChildAsInterior->mpParent = pRoot;
                return true;
                }
            else
                {
                // Create an intermediate node ..
                __RangeTreeInteriorP__ pNewInterior = AllocateInterior ();
                pNewInterior->mpParent = pRoot;
                pRoot->mChild = pNewInterior;
                if (NULL != pOldInterior)
                    InsertChild (pNewInterior, pOldInterior, true);
                InsertChild (pNewInterior, pChild, true);
                return true;
                }
            }
        return false;
        }

    if (pParent->AddChild (pChild))
        {
        if (bFixupRanges)
            {
            RANGETYPE childRange = pChild->Range ();
            __RangeTreeQueryP__ pCurr = pParent;
            RANGETYPE currRange = childRange;
            while (RangeChangesAsExtended (pCurr, currRange))
                {
                pCurr = pCurr->Parent ();
                }
            }
        if (pParent->IsFull ())
            Split (pParent);
        return true;
        }
    return false;
    }
};



struct SplitCandidate
    {
    int             groupNumber[3];
    RANGETYPE        range; //
    double          min;   // active axis min
    double          max;   // active axis max
    __RangeTreeQueryP__  pNode;
    };

static int     cb_qsort_compareForSeparator
(
const SplitCandidate  *pFirst,
const SplitCandidate  *pSecond
)
    {
    if (pFirst->min < pSecond->min)
        return  -1;
    if (pFirst->min > pSecond->min)
        return   1;
    if (pFirst->max < pSecond->max)
        return  -1;
    if (pFirst->max > pSecond->max)
        return   1;
    return  0;
    }




// ===================================================================================
struct __RangeTreeSplitCandidateArray__
    {
    int             numCandidate;
    SplitCandidate  candidate[MAX_TREE_CHILD];

__RangeTreeSplitCandidateArray__ ()
    {
    numCandidate = 0;
    }


bool Add (__RangeTreeQueryP__ pNode)
    {
    // groupNumber and min/max are initialized by algorithms.
    if (numCandidate < MAX_TREE_CHILD)
        {
        candidate[numCandidate].pNode = pNode;
        candidate[numCandidate].range = pNode->Range ();
        numCandidate++;
        return true;
        }
    return false;
    }

void ActivateAxis (int axis)
    {
    if (axis == 0)
        {
        for (int i = 0; i < numCandidate; i++)
            {
            candidate[i].min = candidate[i].range.low.x;
            candidate[i].max = candidate[i].range.high.x;
            }
        }
#ifdef RANGETREE_IS3D
    else if (axis == 1)
#else
    else
#endif
        {
        for (int i = 0; i < numCandidate; i++)
            {
            candidate[i].min = candidate[i].range.low.y;
            candidate[i].max = candidate[i].range.high.y;
            }
        }
#ifdef RANGETREE_IS3D
    else // if (axis == 2)
        {
        for (int i = 0; i < numCandidate; i++)
            {
            candidate[i].min = candidate[i].range.low.z;
            candidate[i].max = candidate[i].range.high.z;
            }
        }
#endif
    }

// Sort along specified axis.
// look for good split point.
// return numeric indicator of how good the split is.
double CheckSeparation
(
int         axis
)
    {
    int minSize = numCandidate / 3;
    ActivateAxis (axis);
    qsort (candidate, numCandidate, sizeof(SplitCandidate),
            (int (*)(const void *,const void *))cb_qsort_compareForSeparator);

    // Look in the middle third for the biggest step forward ...
    int i0 = minSize;                       // First candidate in "middle third"
    int i1 = numCandidate - minSize - 1;    // Last candidate in "middle third"

    int iAfterGap        = -1;    // index of upper entry with largest gap (candidate[i].max to [i+1].min)
    int iAfterAdvance    = -1;    // index of upper entry of non-gap with largest step in simple sort of min's
    double maxGap = 0.0;
    double maxAdvance = -DBL_MAX;
    for (int i = i0; i < i1; i++)
        {
        int j = i + 1;
        double gap = candidate[j].min - candidate[i].max;
        if (gap > 0.0)   // true gap ..
            {
            if (gap > maxGap)
                {
                maxGap = gap;
                iAfterGap = j;
                }
            }
        else        // non gap
            {
            double advance = candidate[j].min - candidate[i].min;
            if (advance > maxAdvance)
                {
                maxAdvance       = advance;
                iAfterAdvance = j;
                }
            }
        }


    int iSeparator;
    if (iAfterGap >= 0)
        iSeparator = iAfterGap;
    else if (iAfterAdvance > 0)
        iSeparator = iAfterAdvance;
    else
        iSeparator = numCandidate / 2;

    for (int i = 0; i < numCandidate; i++)
       candidate[i].groupNumber[axis] = i > iSeparator ? 1 : 0;

    return  maxGap;
    }

void Distribute
(
__RangeTreeInteriorP__ pNodeA,
__RangeTreeInteriorP__ pNodeB,
int selector
)
    {
    pNodeA->InitChildren ();
    pNodeB->InitChildren ();

    // Composite range of two nodes after split is same as range of original, so
    // parent range is unaffected.
    RANGETYPE rangeA, rangeB;
    Init (rangeA);
    Init (rangeB);

    for (int i=0; i < numCandidate; i++)
        {
        if (candidate[i].groupNumber[selector] == 0)
            {
            pNodeA->AddChild (candidate[i].pNode);
            Extend (rangeA, candidate[i].range);
            }
        else
            {
            pNodeB->AddChild (candidate[i].pNode);
            Extend           (rangeB, candidate[i].range);
            }
        }

    pNodeA->SetRange (rangeA);
    pNodeB->SetRange (rangeB);
    }
};

void __RangeTreeFunctions__::Split
(
__RangeTreeInteriorP__ pNodeA
)
    {
    __RangeTreeSplitCandidateArray__ splitCandidates = __RangeTreeSplitCandidateArray__();
    int numChild = pNodeA->mNumChild;
    for (int i = 0; i < numChild; i++)
        splitCandidates.Add (pNodeA->mChild[i]);

    double xSep = splitCandidates.CheckSeparation (0);
    int optimalSplit = 0;
    double optimalSep = xSep;

    double ySep = splitCandidates.CheckSeparation (1);
    if (ySep > optimalSep)
        {
        optimalSep = ySep;
        optimalSplit = 1;
        }
#ifdef RANGETREE_IS3D
    double zSep = splitCandidates.CheckSeparation (2);
    if (zSep > optimalSep)
        {
        optimalSep = zSep;
        optimalSplit = 2;
        }
#endif

    __RangeTreeInteriorP__ pNodeB = AllocateInterior ();
    splitCandidates.Distribute (pNodeA, pNodeB, optimalSplit);
    // pNodeA is already in the parent ...
    InsertChild (pNodeA->Parent (), pNodeB, false);
    }

bool __RangeTreeRoot__::Add (__RangeTreeLeafData__ data, RANGETYPE const&range)
    {
    __RangeTreeLeafP__  pNewLeaf = new __RangeTreeLeaf__ (data, range);
    __RangeTreeInteriorP__ pParent = __RangeTreeFunctions__::FindOrAddParentFor (range, this, mChild);
    return __RangeTreeFunctions__::InsertChild (pParent, pNewLeaf, true);
    }


END_BENTLEY_GEOMETRY_NAMESPACE
