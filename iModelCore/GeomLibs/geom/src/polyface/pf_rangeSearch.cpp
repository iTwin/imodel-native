/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Bentley/BeTimeUtilities.h>


#include <Geom/XYZRangeTree.h>





BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct PolyfaceRangeSearcher : XYZRangeTreeHandler
{
private:
DRange3d m_baseRange;

DRange3d m_currentRange;
bvector<size_t> m_hits;
public:

PolyfaceRangeSearcher ()
    {
    }

~PolyfaceRangeSearcher ()
    {
    }


public:
    size_t m_leafHit;
    size_t m_leafSkip;
    size_t m_subtreeHit;
    size_t m_subtreeSkip;

void ClearCounts ()
    {
    m_leafHit       = 0;
    m_leafSkip      = 0;
    m_subtreeHit    = 0;
    m_subtreeSkip   = 0;
    }


void SetBaseRange (DRange3dCR range)
    {
    m_baseRange = m_currentRange = range;
    }
void SetCurrentRangeFromExpandedBaseRange (double expansion)
    {    
    m_currentRange = m_baseRange;
    m_currentRange.low.x -= expansion;         m_currentRange.high.x += expansion;
    m_currentRange.low.y -= expansion;         m_currentRange.high.y += expansion;
    m_currentRange.low.z -= expansion;         m_currentRange.high.z += expansion;
    }

void ClearHits (){m_hits.clear ();}
void AddHit (size_t hit){m_hits.push_back (hit);}
void CopyHits (bvector<size_t> &hits){hits = m_hits;}

bool ShouldRecurseIntoSubtree (XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior) override 
    {
    DRange3d nodeRange = pInterior->Range ();
    if (m_currentRange.IntersectsWith (nodeRange))
        {
        m_subtreeHit++;
        return true;
        }
    else
        {
        m_subtreeSkip++;
        return false;
        }
    }

bool ShouldContinueAfterSubtree      (XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior)    override {return true;}

bool ShouldContinueAfterLeaf         (XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior, XYZRangeTreeLeafP pLeaf) override 
    {
    size_t leafIndex = (size_t)pLeaf->GetData ();
    DRange3d nodeRange = pLeaf->Range ();
    if (m_currentRange.IntersectsWith (nodeRange))
        {
        AddHit (leafIndex); 
        m_leafHit++;
        }
    else
        {
        m_leafSkip++;
        }
    return true;
    }

};

//================================================================================

PolyfaceRangeTree::PolyfaceRangeTree ()
    : m_rangeTree (NULL)
    {
    }

PolyfaceRangeTree::~PolyfaceRangeTree ()  {ReleaseMem (false);}
void PolyfaceRangeTree::ReleaseMem (bool allocateEmptyTree)
    {
    if (NULL != m_rangeTree)
        XYZRangeTreeRoot::Free (m_rangeTree);
    if (allocateEmptyTree)
        m_rangeTree = XYZRangeTreeRoot::Allocate ();
    else
        m_rangeTree = NULL;
    }

size_t PolyfaceRangeTree::LoadPolyface (PolyfaceQueryCR source)
    {
    ReleaseMem (true);
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (source, false);
    size_t numFacet = 0;
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        size_t readIndex = visitor->GetReadIndex ();
        DRange3d range = DRange3d::From (visitor->Point());
        m_rangeTree->Add ((void*)readIndex, range);
        numFacet++;
        }
    return numFacet;
    }    

PolyfaceRangeTreePtr PolyfaceRangeTree::CreateForPolyface (PolyfaceQueryCR source)
    {
    PolyfaceRangeTree *rangeTree = new PolyfaceRangeTree ();
    rangeTree->LoadPolyface (source);
    return rangeTree;
    }

void PolyfaceRangeTree::CollectInRange (bvector<size_t> &hits, DRange3dCR range, double expansion)
    {
    PolyfaceRangeSearcher searcher;
    searcher.SetBaseRange (range);
    searcher.SetCurrentRangeFromExpandedBaseRange (expansion);
    m_rangeTree->Traverse (searcher);
    searcher.CopyHits (hits);
    }

GEOMDLLIMPEXP XYZRangeTreeRootP PolyfaceRangeTree::GetXYZRangeTree ()
    {
    return m_rangeTree;
    }

XYZRangeTreeRecursionState::XYZRangeTreeRecursionState
(
XYZRangeTreeInteriorP left,
int leftIndex,
XYZRangeTreeInteriorP right,
int rightIndex
) : m_left (left), m_leftIndex (leftIndex), m_right (right), m_rightIndex (rightIndex)
    {
    }

void XYZRangeTreeMultiSearch::Push (XYZRangeTreeInteriorP left, int leftIndex, XYZRangeTreeInteriorP right, int rightIndex)
    {
    if (left != NULL && right != NULL)
        m_stack.push_back (XYZRangeTreeRecursionState (left, leftIndex, right, rightIndex));
    }

void XYZRangeTreeMultiSearch::RunSearch(XYZRangeTreeRootP treeA, XYZRangeTreeRootP treeB, DRange3dPairRecursionHandler &tester)
    {
    m_stack.clear ();
    if (tester.TestInteriorInteriorPair (treeA->mChild->Range (), treeB->mChild->Range ()))
        Push (treeA->mChild, 0, treeB->mChild, 0);

    XYZRangeTreeInteriorP leftInterior, rightInterior;
    XYZRangeTreeLeafP leftLeaf, rightLeaf;

    XYZRangeTreeQueryP leftChild, rightChild;
    // Each tree is a combination of interior and leaf nodes.
    // Each interior node can have any combination of children.
    //    (Well, actually maybe not "any combination" -- one interior node's children are probably uniformly 
    //    leaf or uniformly interior.  But we don't count on that.)
    // The top stack frame points to a interior node in each tree, and has an integer index showing progress through that
    //    node.
    // For a particular pair of left, right interiors, the loop advances the left and right interior indices
    //    in "right fastest" order.
    // Hence (1) when the left index advances out of bounds, the pair is done
    //       (2) when the left is ok but the right has advanced out of bounds, advance left and restart right at 0.
    //       (3) when both are inbounds test the leftChild, rightChild pair as (leaf, leaf), (leaf, interior), (interior,leaf), or (interior,interior)
    //            Push to stack as needed.
    while (tester.StillSearching () && !m_stack.empty ())
        {
        XYZRangeTreeRecursionState state = m_stack.back ();
        if (!state.m_left->TryGetChild (state.m_leftIndex, leftChild))
            {
            m_stack.pop_back ();
            }
        else
            {
            if (!state.m_right->TryGetChild (state.m_rightIndex, rightChild))
                {
                // PROGRESS BY LEFT ADVANCE
                m_stack.back ().m_leftIndex++;
                m_stack.back ().m_rightIndex = 0;
                }
            else
                {
                m_stack.back ().m_rightIndex++;      // PROGRESS BY RIGHT MAJOR ADVANCE (which may trigger left advance on next touch of this stack frame)
                leftLeaf = leftChild->AsLeaf ();
                if (NULL != leftLeaf)
                    {
                    if (NULL != (rightLeaf = rightChild->AsLeaf ()))
                        {
                        tester.TestLeafLeafPair (
                            leftLeaf->Range (), (size_t)leftLeaf->GetData (),
                            rightLeaf->Range (), (size_t)rightLeaf->GetData ()
                            );
                        }
                    else if (NULL != (rightInterior = rightChild->AsInterior ()))
                        {
                        if (tester.TestLeafInteriorPair (
                            leftLeaf->Range (), (size_t)leftLeaf->GetData (), rightInterior->Range ()))
                            Push (state.m_left, state.m_leftIndex, rightInterior, 0);
                        }
                    }
                else if (NULL != (leftInterior = leftChild->AsInterior ()))
                    {
                    if (NULL != (rightLeaf = rightChild->AsLeaf ()))
                        {
                        if (tester.TestInteriorLeafPair (
                            leftInterior->Range (), rightLeaf->Range (), (size_t)rightLeaf->GetData ()))
                            Push (leftInterior, 0, state.m_right, state.m_rightIndex);
                        }
                    else if (NULL != (rightInterior = rightChild->AsInterior ()))
                        {
                        if (tester.TestInteriorInteriorPair (leftInterior->Range (), rightInterior->Range ()))
                            Push (leftInterior, 0, rightInterior, 0);
                        }
                    }
                }
            }
        }
    }

// Collect indices of intersecting facets.
//
// 
struct FacetClashCollector : DRange3dPairRecursionHandler
{
PolyfaceVisitorPtr m_visitorA;
PolyfaceVisitorPtr m_visitorB;

PolyfaceQueryR  m_polyfaceA;
PolyfaceQueryR m_polyfaceB;

BoolCounter m_II;
BoolCounter m_IL;
BoolCounter m_LI;
BoolCounter m_LL;
BoolCounter m_LLClash;

bvector<std::pair<size_t, size_t>> &m_hits;
size_t m_maxHits;
double m_rangeExpansion;
double m_proximity;



FacetClashCollector
(
PolyfaceQueryR polyfaceA,
PolyfaceQueryR polyfaceB,
double rangeExpansion,
double proximity,
bvector<std::pair<size_t, size_t>> &hits,
size_t maxHits
)
: m_rangeExpansion (rangeExpansion),
  m_polyfaceA (polyfaceA),
  m_polyfaceB (polyfaceB),
  m_hits (hits),
  m_maxHits (maxHits),
  m_proximity (proximity)
    {
    m_visitorA = PolyfaceVisitor::Attach (polyfaceA);
    m_visitorB = PolyfaceVisitor::Attach (polyfaceB);
    m_visitorA->Reset ();
    m_visitorB->Reset ();
    if (m_rangeExpansion < proximity)
        m_rangeExpansion = proximity;
    }
    
bool TestOverlap (DRange3dCR rangeA, DRange3dCR rangeB)
    {
    return rangeA.IntersectsWith (rangeB, m_rangeExpansion, 3);
    }

bool TestInteriorInteriorPair (DRange3dCR rangeA, DRange3dCR rangeB) override 
  {
  return m_II.Count (TestOverlap (rangeA, rangeB));
  }

bool TestInteriorLeafPair (DRange3dCR rangeA, DRange3dCR rangeB, size_t indexB) override
  {
  return m_IL.Count (TestOverlap (rangeA, rangeB));
  }

bool TestLeafInteriorPair (DRange3dCR rangeA, size_t indexA, DRange3dCR rangeB) override
  {
  return m_IL.Count (TestOverlap (rangeA, rangeB));
  }

void TestLeafLeafPair (DRange3dCR rangeA, size_t indexA, DRange3dCR rangeB, size_t indexB) override
    {
    if (m_LL.Count (rangeA.IntersectsWith (rangeB, m_rangeExpansion, 3)))
        {
        m_visitorA->MoveToFacetByReadIndex (indexA);
        m_visitorB->MoveToFacetByReadIndex (indexB);
        if (m_proximity <= 0.0)
            {
        if (m_LLClash.Count (bsiDPoint3dArray_polygonClashXYZ (
            m_visitorA->GetPointCP (), (int) m_visitorA->Point ().size (),
            m_visitorB->GetPointCP (), (int) m_visitorB->Point ().size ()
            )))
                {
                m_hits.push_back (std::pair <size_t, size_t> (indexA, indexB));
                }
            }
        else
            {
            DPoint3d pointA, pointB;
            if (m_LLClash.Count (bsiPolygon_closestApproachBetweenPolygons (
                        &pointA, &pointB,
                        m_visitorA->GetPointCP (), (int) m_visitorA->Point ().size (),
                        m_visitorB->GetPointCP (), (int) m_visitorB->Point ().size ()
                    )))
                {
                if (pointA.DistanceSquared (pointB) < m_proximity * m_proximity)
            m_hits.push_back (std::pair <size_t, size_t> (indexA, indexB));
                }            
            }
        }
    }

bool StillSearching () override
    {
    return m_hits.size () < m_maxHits;
    }
};    
    
// Search for clashing pairs.
//
GEOMDLLIMPEXP void PolyfaceRangeTree::CollectClashPairs (
PolyfaceQueryR polyfaceA,           //!< first polyface
PolyfaceRangeTree &treeA,           //!< range tree for polyfaceA
PolyfaceQueryR polyfaceB,           //!< second polyface
PolyfaceRangeTree &treeB,           //!< range tree for polyfaceB
double proximity,                   
bvector<std::pair<size_t, size_t>> &hits,   //!< read indices of clashing pairs
size_t maxHits,                      //! maximum number of hits to collect.
XYZRangeTreeMutltiSearchR searcher    //! (to be reused over multiple calls)
)
    {
    hits.clear ();
    double rangeExpansion = proximity + 1.0e-12;
    FacetClashCollector collector (polyfaceA, polyfaceB, rangeExpansion, proximity, hits, maxHits);
    searcher.RunSearch (treeA.GetXYZRangeTree (), treeB.GetXYZRangeTree (), collector);
    }





END_BENTLEY_GEOMETRY_NAMESPACE