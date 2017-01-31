/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/BinaryRangeHeap.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/
/*__PUBLISH_SECTION_START__*/
/*__PUBLISH_SECTION_END__*/

#include <algorithm>
BEGIN_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE



struct RangeSplitter
{
bvector<DoubleSizeSize> sortArray;

// Scan the array of sourceSelectors.
// At each entry that matches selector, update numSelected, selectedUnion, and selectedIntersection.
//
GEOMDLLIMPEXP static void GetSelectedRangeStatistics
(
bvector <DRange3d> const &sourceRange,
bvector<int> const &sourceSelector,
int selector,
size_t &numSelected,
DRange3d &selectedUnion,
DRange3d &selectedIntersection
);
//! Determine distribution of left, center, and right ranges to maximize {(1+numLeft)(1+numRight)(1+numCenter)}.
//! 
//! @param [out] numLeft number of ranges strictly to left of split.
//! @param [out] numCenter number of ranges that have overlap.
//! @param [out] numRight number of ranges that are strictly to right of split.
GEOMDLLIMPEXP bool SplitRanges
(
bvector<DRange3d> const &sourceRange,
int axisSelect,
bvector<int> &leftCenterRight,
size_t &numLeft,
size_t &numCenter,
size_t &numRight
);

};


struct ITernaryRangeTree;
typedef RefCountedPtr<ITernaryRangeTree>  ITernaryRangeTreePtr;

typedef struct ITernaryRangeTree *ITernaryRangeTreeP;
typedef struct ITernaryRangeTree &ITernaryRangeTreeR;

struct ITernaryRangeTree : public RefCountedBase
{
protected:
//! Add a range to an unsorted list that can be efficiently reorganized later.
GEOMAPI_VIRTUAL void _Preload (DRange3dCR range, size_t tag) = 0;
//! Distribute preloaded ranges into a search structure.
GEOMAPI_VIRTUAL void _DistributePreloads (size_t maxPerNode) = 0;
//! Get the index of the root node.
GEOMAPI_VIRTUAL size_t _GetRoot () const = 0;
//! Get index of left  children of nodeIndex;
GEOMAPI_VIRTUAL size_t _GetLeftChild (size_t nodeIndex) const = 0;
GEOMAPI_VIRTUAL size_t _GetRightChild (size_t nodeIndex) const = 0;
GEOMAPI_VIRTUAL size_t _GetMidChild (size_t nodeIndex) const = 0;

//! Copy all the ranges from a node.
GEOMAPI_VIRTUAL void _GetRangesInNode
(
size_t nodeIndex,
bool recurseToChildren,
bvector<DRange3dSizeSize> &data
) const = 0;
public:

static GEOMDLLIMPEXP ITernaryRangeTreePtr Create ();
//! Add a range to an unsorted list that can be efficiently reorganized later.
void GEOMDLLIMPEXP Preload (DRange3dCR range, size_t tag);
//! Distribute preloaded ranges into a search structure.
void GEOMDLLIMPEXP DistributePreloads (size_t maxPerNode);
//! Get the index of the root node.
size_t GEOMDLLIMPEXP GetRoot () const;
//! Get index of left  children of nodeIndex;
size_t GEOMDLLIMPEXP GetLeftChild (size_t nodeIndex) const;
size_t GEOMDLLIMPEXP GetRightChild (size_t nodeIndex) const;
size_t GEOMDLLIMPEXP GetMidChild (size_t nodeIndex) const;

//! Copy all the ranges from a node.
//! @param [in] nodeIndex top of tree.
//! @param [in] recurseToChildren true to continue to children.
//! @param [out] data vector to receive ranges.
void GEOMDLLIMPEXP GetRangesInNode
(
size_t nodeIndex,
bool recurseToChildren,
bvector<DRange3dSizeSize> &data
) const;
};


struct IndexedRangeHeap;
typedef IndexedRangeHeap const & IndexedRangeHeapCR;
typedef IndexedRangeHeap const * IndexedRangeHeapCP;
typedef IndexedRangeHeap & IndexedRangeHeapR;
typedef IndexedRangeHeap * IndexedRangeHeapP;

//! Search structure for a collection of ranges made available "by index" by calls to a query object.
//! The heap is based on left-right splitting of the INDICES (not the ranges).
//! Each heap node carries both (a) lower and upper indices and (b) the composite range of data at those indices.
struct IndexedRangeHeap
{
public:
// Interface for object that can be queried to get original range data in given index range:
    struct IndexedRangeSource
    {
    // Return the range for indices i0 through i1 inclusive.  Return false if no data for these indices.
    GEOMAPI_VIRTUAL bool GetRange (size_t i0, size_t i1, DRange3d &range) const = 0;
    };

// Interface for two-heap searches.
    struct PairProcessor
    {
    // Ask if processing is needed for given ranges and indices from two sources.
    GEOMAPI_VIRTUAL bool NeedProcessing (
            DRange3dCR rangeA, size_t iA0, size_t iA1,
            DRange3dCR rangeB, size_t iB0, size_t iB1
            ) = 0;
    // Announce a final index pair that has passed all tests.
    GEOMAPI_VIRTUAL void Process (size_t iA, size_t iB) = 0;
    // Ask if search is still active.
    GEOMAPI_VIRTUAL bool IsLive () const = 0;
    };

// Interface for single heap search
    struct SingleProcessor
    {
    // Ask if processing is needed for given range and indices
    GEOMAPI_VIRTUAL bool NeedProcessing (
            DRange3dCR rangeA, size_t iA0, size_t iA1
            ) = 0;
    // Announce a final index that has passed all tests.
    GEOMAPI_VIRTUAL void Process (size_t iA) = 0;
    // Ask if search is still active.
    GEOMAPI_VIRTUAL bool IsLive () const = 0;
    };

// Base class for single heap search by point containment.
// Derived class must implement Process(size_t) but all others are implemented.
    struct XYZSearchBase : SingleProcessor
    {
    DPoint3d m_xyz;
    int m_numDimensions;   // 1, 2, or 3 dimensions XYZ
    XYZSearchBase (DPoint3dCR xyz, int numDimensions = 3) : m_xyz(xyz), m_numDimensions (numDimensions) {}
    void SetSearchXYZ (DPoint3dCR xyz, int numDimensions = 3)
        {
        m_xyz = xyz;
        m_numDimensions = numDimensions;
        }

    bool IsLive () const override {return true;}
    bool NeedProcessing (DRange3dCR range, size_t iA0, size_t iA1) override
        {
        return range.IsContained (m_xyz, m_numDimensions);
        };
    };

// Base class for two-heap search by range intersection
// Derived class must implement Process(size_t, size_t) but all others are implemented.
    struct RangeIntersectionSearchBase : PairProcessor
    {
    int m_numDimensions;
    RangeIntersectionSearchBase (int numDimensions) : m_numDimensions (numDimensions) {}
    bool IsLive () const override {return true;}
    bool NeedProcessing (DRange3dCR rangeA, size_t iA0, size_t iA1,
                            DRange3dCR rangeB, size_t iB0, size_t iB1) override
        {
        return rangeA.IntersectsWith (rangeB, m_numDimensions);
        };
    };


private:
  struct RangeEntry
    {
    DRange3d m_range;
    size_t   m_i0;
    size_t   m_i1;
    size_t   m_child;  // First of 2 consecutive entries that are children of this node.
    RangeEntry ();  // Initialize as null entry with invalid indices.
    RangeEntry (DRange3dCR range, size_t i0 = SIZE_MAX, size_t i1 = SIZE_MAX, size_t child = SIZE_MAX);
    void Set (DRange3dCR range);
    DRange3d Get () const;
    };

bvector <RangeEntry> m_ranges;
size_t  m_numPerEntry;
IndexedRangeSource const *m_source;

void InitializeThroughIndex (size_t index);
void SetUnevaluatedRange (size_t index, size_t i0, size_t i1);
void CorrectRangeFromChildren (size_t parent);
void SetEvaluatedRange (size_t index, size_t i0, size_t i1);
void SetRecursiveRange (size_t index, size_t i0, size_t i1);
void SetNewChildIndices (size_t index, size_t &child0, size_t &child1);

public:

//! Bbuild a range heap for ranges indexed i0..i1 inclusive.
//! Range is requested only once per index.
//! @param [in] numPerEntry max number of ranges to hold in a single leaf.
//! @param [in] source object that returns ranges on demand.
//! @param [in] i0 first range index.
//! @param [in] i1 last range index.
void GEOMDLLIMPEXP Build (size_t numPerEntry, IndexedRangeSource const *source, size_t i0, size_t i1);

//! Test for valid node index in the heap.
bool GEOMDLLIMPEXP IsValidIndex (size_t index) const;
//! Retrieve the range data at a heap node.
bool GEOMDLLIMPEXP Get (size_t index, DRange3dR range) const;
//! Retrieve the range and range index data at a heap node.
bool GEOMDLLIMPEXP Get (size_t index, DRange3dR range, size_t &i0, size_t &i1) const;
//! Ask if a heap node is a leaf node.
bool GEOMDLLIMPEXP IsLeafIndex (size_t index) const;
//! Get the node index of a child of a heap node.   (In binary heap, the only valid childOffsets are 0 and 1)
bool GEOMDLLIMPEXP GetChildIndex (size_t parent, size_t childOffset, size_t &childIndex) const;
//! Return the index of the root node of the heap.
size_t GEOMDLLIMPEXP GetRootIndex () const;

// Recursively compare subtrees of a pair of range heaps.
// Ask processor as needed whether range and index data needs to be processed.
// Announce final leaf pairs to processor.
static void GEOMDLLIMPEXP Search (IndexedRangeHeapCR heapA, IndexedRangeHeapCR heapB,
    IndexedRangeHeap::PairProcessor &processor); 
// sortMethod=1 chooses child pairs with small diagonal first.
static void GEOMDLLIMPEXP Search (IndexedRangeHeapCR heapA, IndexedRangeHeapCR heapB,
    IndexedRangeHeap::PairProcessor &processor, int sortMethod); 


// Recursively search subtrees a range heap.
// Ask processor as needed whether range and index data needs to be processed.
// Announce final leaf indices as reached.
static void GEOMDLLIMPEXP Search (IndexedRangeHeapCR heap, IndexedRangeHeap::SingleProcessor &processor);
};

class RangeSortAlgorithms
{
private:
    RangeSortAlgorithms ();  // no instances 
public:
enum class SplitType
    {
    TwoWayMoments,    // Given (n0, n1, n2), maximize n0*n2
    ThreeWayMoments   // Given (n0, n1, n2), maximize n0*n1*n2
    };

    //! generate a list of indices that mimic binary splits.
    //! @param [in] ranges ranges to sort.
    //! @param [out] indices constructed indices
    //! @param [in] sortX true to consider x-direction splits.
    //! @param [in] sortY true to consider y-direction splits
    //! @param [in] sortZ true to consider z-direction splits
    //! @param [in] splitType algorithm selector.
    static GEOMDLLIMPEXP  void PseudoBinarySplits (bvector<DRange3dSizeSize> const &ranges, bvector<size_t> &indices, bool sortX, bool sortY, bool sortZ, SplitType splitType);
};

END_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE
