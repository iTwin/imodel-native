/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/XYZRangeTree.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


struct XYVisibilitySplitter : XYZRangeTreeHandler
{
private:
bvector<DPoint3d> m_currPoints;

bvector<DPoint3d> m_clipperPoints;
bvector<DPoint3d> m_clipperPointsOnPlane;
bvector<double>   m_clipperAltitudes;
double                m_abstol;
bvector<DPoint3d> m_clipperLoop;    // Exracted from m_clipperPointsOnPlane
size_t m_currIndex;

DPlane3d m_currPlane;   // Plane origin and normal, with normal scaled so plane evaluation gives z altitude.

TaggedPolygonVectorCR m_polygons;
XYVisibilityClipQuery &m_clipQuery;
DRange3d      m_searchRange;
DRange3d      m_globalRange;
PolygonMarkedMultiClipP m_clipper;

// Resize the clipper arrays to numPrimaryPoints.
// Replicated max(numCopy0, numCopy1) points at end.
void WrapClipperData (size_t numPrimaryPoints, size_t numCopy0, size_t numCopy1)
    {
    size_t numCopy = 0;
    if (numCopy0 > numCopy)
        numCopy = numCopy0;
    if (numCopy1 > numCopy)
        numCopy = numCopy1;
    m_clipperPoints.resize (numPrimaryPoints);
    m_clipperPointsOnPlane.resize (numPrimaryPoints);
    m_clipperAltitudes.resize (numPrimaryPoints);
    for (size_t i = 0; i < numCopy; i++)
        {
        m_clipperAltitudes.push_back (m_clipperAltitudes[i]);
        // copy point structures out to avoid pointers into resized buffer ...
        DPoint3d xyz = m_clipperPoints[i];
        m_clipperPoints.push_back (xyz);
        xyz = m_clipperPointsOnPlane[i];
        m_clipperPointsOnPlane.push_back (xyz);
        }
    }

void PushInterpolatedPoint (bvector<DPoint3d> &dest,
        bvector<DPoint3d> &source,
        bvector<double> &altitude,
        size_t i0
        )
    {
    size_t i1 = i0 + 1;
    double h0 = altitude[i0];
    double h1 = altitude[i1];
    double fraction;
    if (bsiTrig_safeDivide (&fraction, -h0, h1 - h0, 0.0))
        {
        DPoint3d xyz;
        xyz.Interpolate (source[i0], fraction, source[i1]);
        dest.push_back (xyz);
        }
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

XYVisibilitySplitter (TaggedPolygonVectorCR polygons, double abstol, XYVisibilityClipQuery &clipQuery)
    : m_polygons (polygons),
      m_clipQuery (clipQuery)
    {
    m_globalRange = PolygonVectorOps::GetRange (m_polygons);
    ClearCounts ();
    m_clipper = VuMultiClipFactory::NewPolygonMarkedMultiClip ();
    m_abstol = abstol;
    }

GEOMAPI_VIRTUAL ~XYVisibilitySplitter ()
    {
    VuMultiClipFactory::FreePolygonMarkedMultiClip (m_clipper);
    }

bool Load (size_t index)
    {
    static double s_normalZTolerance = 5.0e-4; /// We really don't want panels that are close to vertical.
    if (index >= m_polygons.size ())
        return false;
    m_currPoints = m_polygons.at(index).GetPointsCR ();
    DPoint3d centroid;
    DVec3d   normal;
    double   area, perimeter, maxPlanarError;
    m_currIndex  = index;
    bool hasRange = PolygonVectorOps::HasNonNullRange (m_polygons, index, m_searchRange);

    if (!bsiPolygon_centroidAreaPerimeter (&m_currPoints[0], (int)m_currPoints.size (),
                &centroid, &normal, &area, &perimeter, &maxPlanarError))
        return false;
    if (fabs (normal.z) < s_normalZTolerance)
        return false;
    double inverseNZ;
    if (!bsiTrig_safeDivide (&inverseNZ, 1.0, normal.z, 0.0))
        return false;
    normal.Scale (inverseNZ);
    m_currPlane.InitFromOriginAndNormal (centroid, normal);

    if (hasRange)
        {
        m_searchRange.high.z = m_globalRange.high.z;
        }

    m_clipper->Initialize ();
    m_clipper->AddBasePolygon (&m_currPoints[0], NULL, (int)m_currPoints.size ());
    return hasRange;
    }


double DistanceSpacePointToPlane (DPoint3dCR spacePoint)
    {
    return spacePoint.DotDifference (m_currPlane.origin, m_currPlane.normal);
    }

DPoint3d ProjectSpacePointToPlane (DPoint3dCR spacePoint)
    {
    double a = DistanceSpacePointToPlane (spacePoint);
    DPoint3d planePoint = spacePoint;
    planePoint.z -= a;
    return planePoint;
    }


void BuildLoops (size_t firstIndexOnOtherSide, size_t numDistinctPoints, double altitudeSign)
    {
    // Find strings of positive altitudes following negatives.
    size_t firstNegativeIndexPlusPeriod = numDistinctPoints + firstIndexOnOtherSide;
    for (size_t i0 = firstIndexOnOtherSide; i0 < firstNegativeIndexPlusPeriod;)
        {
        // i0 is a negative index.
        // Walk forward through nonnegatives.  Because of the wrap, we are assured
        // if finding one (without an index check)
        size_t i1;
        for (i1 = i0 + 1; m_clipperAltitudes[i1] * altitudeSign >= 0.0;)
            {
            i1++;
            }
        if (i1 > i0)
            {
            m_clipperLoop.clear ();
            if (m_clipperAltitudes[i0 + 1] * altitudeSign > 0.0)
                PushInterpolatedPoint (m_clipperLoop, m_clipperPointsOnPlane, m_clipperAltitudes, i0);
            for (size_t i = i0+1; i < i1; i++)
                m_clipperLoop.push_back (m_clipperPointsOnPlane[i]);
            if (m_clipperAltitudes[i1 - 1] * altitudeSign > 0.0)
                PushInterpolatedPoint (m_clipperLoop, m_clipperPointsOnPlane, m_clipperAltitudes, i1 - 1);
            if (m_clipperLoop.size () > 2)
                {
                DPoint3d xyz = m_clipperLoop[0];
                m_clipperLoop.push_back (xyz);
                m_clipper->AddClipper (&m_clipperLoop[0], NULL, (int)m_clipperLoop.size ());  
                }
            i0 = i1;
            }
        }
    }

void ApplyClipper (bvector<DPoint3d> const &clipper,
    XYVisibilityClipQuery::ClipSelections const &selections)
    {
    if (!selections.useBelow && !selections.useOn && !selections.useAbove)
        return;


    m_clipperPoints = clipper;
    m_clipperAltitudes.clear ();
    m_clipperPointsOnPlane.clear ();
    ptrdiff_t firstNegativeIndex = -1;
    ptrdiff_t firstPositiveIndex = -1;
    size_t numPositive = 0;
    size_t numNegative = 0;
    size_t numOn = 0;
    //size_t i = 0;
    size_t numClipperPoints = m_clipperPoints.size ();
    
    for (size_t i = 0; i < numClipperPoints; i++)
        {
        DPoint3d xyz = m_clipperPoints[i];
        double z = DistanceSpacePointToPlane (xyz);
        m_clipperAltitudes.push_back (z);
        m_clipperPoints.push_back (xyz);
        xyz.z -= z;
        m_clipperPointsOnPlane.push_back (xyz);
        if (fabs (z) < m_abstol)
            {
            numOn++;
            }
        if (z >= 0.0)
            {
            numPositive++;
            if (firstPositiveIndex < 0)
                firstPositiveIndex = i;
            }
        else
            {
            numNegative++;
            if (firstNegativeIndex < 0)
                firstNegativeIndex = i;
            }
        }

    if (numOn == numClipperPoints)
        {
        // Coplanar polygons ...
        if (selections.useOn)
            m_clipper->AddClipper (&m_clipperPointsOnPlane[0], NULL, (int)m_clipperPointsOnPlane.size ());
        }
    else if (numPositive == 0)
        {
        // Clipper is entirely behind.
        if (selections.useBelow)
            m_clipper->AddClipper (&m_clipperPointsOnPlane[0], NULL, (int)m_clipperPointsOnPlane.size ());
        }
    else if (numNegative == 0)
        {
        // Everything is in front.
        if (selections.useAbove)
            m_clipper->AddClipper (&m_clipperPointsOnPlane[0], NULL, (int)m_clipperPointsOnPlane.size ());
        }
    else if (selections.useAbove && selections.useBelow)
        {
        m_clipper->AddClipper (&m_clipperPointsOnPlane[0], NULL, (int)m_clipperPointsOnPlane.size ());        
        }
    else if (selections.useAbove || selections.useBelow)
        {
        size_t numDistinctPoints = numClipperPoints;
        if (m_clipperPoints[0].IsEqual (m_clipperPoints[numClipperPoints - 1]))
            numDistinctPoints--;
        WrapClipperData (numDistinctPoints, firstNegativeIndex + 1, firstPositiveIndex + 1);
        if (selections.useAbove)
            BuildLoops (firstNegativeIndex, numDistinctPoints, 1.0);
        if (selections.useBelow)
            BuildLoops (firstPositiveIndex, numDistinctPoints, -1.0);
        }
    }



bool ShouldRecurseIntoSubtree(XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior) override {
    DRange3d nodeRange = pInterior->Range ();
    if (m_searchRange.IntersectsWith (nodeRange))
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

bool ShouldContinueAfterSubtree(XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior) override {return true;}

bool ShouldContinueAfterLeaf(XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior, XYZRangeTreeLeafP pLeaf) override {
    size_t leafIndex = (size_t)pLeaf->GetData ();
    if (leafIndex != m_currIndex)
        {
        DRange3d nodeRange = pLeaf->Range ();
        if (m_searchRange.IntersectsWith (nodeRange))
            {
            ApplyClipper (m_polygons[leafIndex].GetPointsCR (),
                        m_clipQuery.GetClipAction (m_polygons, m_currIndex, leafIndex));
            m_leafHit++;
            }
        else
            {
            m_leafSkip++;
            }
        }
    return true;
    }

void CollectClip (TaggedPolygonVectorR outputPolygons, bool primarySelect = true, bool secondarySelect = false)
    {
    m_clipper->FinishClip (PolygonMarkedMultiClip::FaceMode_Triangulate);
    #define MAXOUT 1000
    DPoint3d xyz[MAXOUT];
    int numThisFace;

    m_clipper->SetupForLoopOverFaces (primarySelect, secondarySelect);
    size_t numPolygon = 0;
    for (;m_clipper->GetFace (xyz, NULL, numThisFace, MAXOUT, false);)
        {
        PolygonVectorOps::AddPolygon (outputPolygons, xyz, numThisFace);
        if (m_currIndex < m_polygons.size ())
            outputPolygons.at (outputPolygons.size () - 1).CopyTagsFrom (m_polygons.at (m_currIndex));
        numPolygon ++;
        }
    }
};

struct MyTimer
{
#ifdef ImplementTimer
LARGE_INTEGER m_startTime;
LONGLONG m_accumulator;
#else
size_t m_accumulator;
#endif
double m_publicTime;
MyTimer () : m_accumulator(0)
    {
#ifdef ImplementTimer
    m_startTime.QuadPart = 0;
#endif
    }
void Clear () {m_accumulator = 0;}

void Start ()
    {
#ifdef ImplementTimer
    QueryPerformanceCounter (&m_startTime);
#endif
    }
void Finish ()
    {
#ifdef ImplementTimer
    LARGE_INTEGER endTime;
    QueryPerformanceCounter (&endTime);
    m_accumulator += (endTime.QuadPart - m_startTime.QuadPart);
#endif
    }

double FractionOf (MyTimer &parent)
    {
    if (parent.m_accumulator > 0)
        return (double)m_accumulator / (double)parent.m_accumulator;
    return 0.0;
    }
};

GEOMDLLIMPEXP void bsiPolygon_clipByXYVisibility
(
TaggedPolygonVectorCR source,
TaggedPolygonVectorR dest,
bool select1,
bool select2,
XYVisibilityClipQuery &actionSelector
)
    {
    dest.clear ();

    DRange3d totalRange = PolygonVectorOps::GetRange (source);
    double abstol = bsiTrig_smallAngle () * totalRange.LargestCoordinate ();
    XYZRangeTreeRootP rangeTree = XYZRangeTreeRoot::Allocate ();
    size_t n = source.size ();

#ifdef PRINT_STATS
    if (s_debug > 2)
        printf ("(polygons %d\n", n);
#endif

    XYVisibilitySplitter splitter (source, abstol, actionSelector);
    for (size_t i = 0; i < n; i++)
        {
        DRange3d range;
        if (PolygonVectorOps::HasNonNullRange (source, i, range))
            {
            rangeTree->Add ((void*)i, range);
            }
        }

    MyTimer totalSearchTime;
    MyTimer traverseTime;
    MyTimer collectTime;
    MyTimer loadTime;
    XYZRangeTreeCounter counter = XYZRangeTreeCounter ();
    rangeTree->Traverse (counter);
#ifdef PRINT_STATS
    printf ("(RangeTree (#leaf %d) (#fringe %d) (#interior %d) (#maxDepth %d) (sumLeafSquared %ld) \n",
                    counter.mNumLeaf, counter.mNumFringe,
                    counter.mNumInterior, counter.mMaxDepth,
                    counter.mSumFringeLeafCountSquared
                    );
#endif
    static ptrdiff_t s_singlePolygonSelector = -1;
    if (s_singlePolygonSelector >= 0 && s_singlePolygonSelector < (ptrdiff_t)n)
        {
        splitter.Load (s_singlePolygonSelector);
        rangeTree->Traverse (splitter);
        splitter.CollectClip (dest, select1, select2);
        }
    else
        {
        totalSearchTime.Start ();
        splitter.ClearCounts ();
        for (size_t i = 0; i < n; i++)
            {
            loadTime.Start ();
            bool loaded = splitter.Load (i);
            loadTime.Finish ();
            if (loaded)
                {
                traverseTime.Start ();
                rangeTree->Traverse (splitter);
                traverseTime.Finish ();
                collectTime.Start ();
                splitter.CollectClip (dest, select1, select2);
                collectTime.Finish ();
                }
            }
        totalSearchTime.Finish ();
    #ifdef PRINT_STATS
            printf ("(Subtree %d %d) (Leaf %d %d)\n",
                        splitter.m_subtreeHit,
                        splitter.m_subtreeSkip,
                        splitter.m_leafHit,
                        splitter.m_leafSkip
                        );
    #endif
        }

    //double loadFraction = loadTime.FractionOf (totalSearchTime);
    //double traverseFraction = traverseTime.FractionOf (totalSearchTime);
    //double collectFraction = collectTime.FractionOf (totalSearchTime);
    XYZRangeTreeRoot::Free (rangeTree);
    }


XYVisibilityClipQuery::ClipSelections::ClipSelections
(
bool _useBelow,
bool _useOn,
bool _useAbove
) : useBelow (_useBelow),
    useOn (_useOn),
    useAbove (_useAbove)
    {
    }

struct DefaultXYVisibilityClipQuery : XYVisibilityClipQuery
{
//! Ask how to source[clipperIndex] clips source[clippeeIndex]
//! @param [out] selections  returned actions for clipper portions below, on, and above.
ClipSelections GetClipAction
(
TaggedPolygonVectorCR source,
size_t  clippeeIndex,
size_t  clipperIndex
) override
    {
    return ClipSelections (false, clipperIndex < clippeeIndex, true);
    }
};

GEOMDLLIMPEXP void bsiPolygon_clipByXYVisibility
(
TaggedPolygonVectorCR source,
TaggedPolygonVectorR dest,
bool select1,
bool select2
)
    {
    DefaultXYVisibilityClipQuery selector;
    return bsiPolygon_clipByXYVisibility (source, dest, select1, select2, selector);
    }

END_BENTLEY_GEOMETRY_NAMESPACE