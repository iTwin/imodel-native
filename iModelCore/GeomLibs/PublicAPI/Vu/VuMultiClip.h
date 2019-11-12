/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/


BEGIN_BENTLEY_GEOMETRY_NAMESPACE

class   PolygonMarkedMultiClip;
typedef PolygonMarkedMultiClip* PolygonMarkedMultiClipP;


//! @description A TaggedPolygon is a vector of DPoint3d with a PolygonTag.
//!
struct TaggedPolygon
{
private:
    class PolygonTag    // Must be CLASS not STRUCT to compile implementation
        {
        public:
        PolygonTag ();
        PolygonTag (ptrdiff_t indexA, ptrdiff_t indexB = 0, double a = 0.0);
        ptrdiff_t m_indexA;
        ptrdiff_t m_indexB;
        double m_a;
        };
PolygonTag m_tag;
bvector<DPoint3d> m_points;
public:

//! @return indexA from PolygonTag
GEOMDLLIMPEXP ptrdiff_t GetIndexA () const;
//! @return indexB from PolygonTag
GEOMDLLIMPEXP ptrdiff_t GetIndexB () const;
//! @return double from PolygonTag
GEOMDLLIMPEXP double    GetTag () const;


//! Set indexA in PolygonTag
GEOMDLLIMPEXP void SetIndexA (ptrdiff_t indexA);
//! Set indexB in PolygonTag
GEOMDLLIMPEXP void SetIndexB (ptrdiff_t indexB);
//! Set double in PolygonTag
GEOMDLLIMPEXP void SetTag (double tag);

//! @return Reference to contained vector of points.
GEOMDLLIMPEXP bvector<DPoint3d> & GetPointsR ();
//! @return Const reference to contained vector of points.
GEOMDLLIMPEXP bvector<DPoint3d> const & GetPointsCR () const;
//! @return size of vector.
GEOMDLLIMPEXP size_t GetPointSize () const;
//! @return Simple pointer to points.
GEOMDLLIMPEXP DPoint3dP GetDataP ();
//! @return Simple const pointer to points.
GEOMDLLIMPEXP DPoint3dCP GetDataCP () const;
//! @return bounds-checked access to a single point.
GEOMDLLIMPEXP bool TryGetPoint (size_t index, DPoint3dR value) const;
//! @return bounds-checked access to single point with cyclic indexing.
//! @remark Fails only for empty array or zero period.
GEOMDLLIMPEXP bool TryGetCyclicPoint (size_t index, DPoint3dR value) const;

GEOMDLLIMPEXP size_t GetTrimmedSize (double tol) const;

GEOMDLLIMPEXP DRange3d GetRange () const;

//! Copy all tags from source.
GEOMDLLIMPEXP void CopyTagsFrom (TaggedPolygonCR source);

//! Clear points
GEOMDLLIMPEXP void Clear ();
//! Add single point
GEOMDLLIMPEXP void Add (DPoint3dCR xyz);
//! Add single point
GEOMDLLIMPEXP void Add (double x, double y, double z);

//! Multiply by transform
GEOMDLLIMPEXP void Multiply (TransformCR transform);

//! Constructor from vector and (optional) listed tag contents.
GEOMDLLIMPEXP TaggedPolygon (bvector<DPoint3d> points, ptrdiff_t indexA = 0, ptrdiff_t indexB = 0, double tag = 0.0);
//! Constructor from pointer to buffer and (optional) listed tag contents.
GEOMDLLIMPEXP TaggedPolygon (DPoint3dCP points, size_t n,  ptrdiff_t indexA = 0, ptrdiff_t indexB = 0, double tag = 0.0);
//! Empty constructor
GEOMDLLIMPEXP TaggedPolygon ();
//! Constructor for tags with no points.
GEOMDLLIMPEXP TaggedPolygon (ptrdiff_t indexA, ptrdiff_t indexB = 0, double tag = 0.0);

// Swap contents without forcing reallocation.
GEOMDLLIMPEXP void Swap (TaggedPolygon &other);

// Return area when viewed in the xy plane.
GEOMDLLIMPEXP double AreaXY () const;
// Return area when viewed in the plane perpendicular to given vector.
GEOMDLLIMPEXP double Area (DVec3dCR perpendicularVector) const;

};

struct GEOMDLLIMPEXP PolygonVectorOps  // NonInstantiable, static operations on PolygonVector
{
private:
PolygonVectorOps (){}
public:


//! push a (copy of a) polygon onto the polygon list.
static void AddPolygon (TaggedPolygonVectorR dest,
            bvector <DPoint3d> const &points,
            ptrdiff_t indexA = 0, ptrdiff_t indexB = 0, double a = 0.0
            );
//! push a (copy of a) polygon onto the polygon list.
static void AddPolygon (TaggedPolygonVectorR dest,
            DPoint3dCP points, int n,
            ptrdiff_t indexA = 0, ptrdiff_t indexB = 0, double a = 0.0
            );

//! push polygon onto the polygon list.
//! The caller's bvector heap memory is used (moved) without allocation.
static void AddPolygonCapture (TaggedPolygonVectorR dest,
            bvector <DPoint3d> &points,
            ptrdiff_t indexA = 0, ptrdiff_t indexB = 0, double a = 0.0
            );
//! push polygon with vector shift.
static void AddTransformedPolygon (TaggedPolygonVectorR dest, bvector <DPoint3d> &points,
         DVec3dCR shift,
         ptrdiff_t indexA = 0, ptrdiff_t indexB = 0, double a = 0.0);

//! return range of all polygons
static DRange3d GetRange (TaggedPolygonVectorCR source);
//! return range of a single polygon
static DRange3d GetRange (TaggedPolygonVectorCR source, size_t i);

//! total points over all members
static size_t GetTotalPointCount (TaggedPolygonVectorCR source);

//! sum xy areas over all members.
static double GetSummedAreaXY (TaggedPolygonVectorCR source);
//! test if a single polygon has non null range
static bool HasNonNullRange (TaggedPolygonVectorCR source, size_t i, DRange3dR range);
//! Multiply all points by transform.
static void Multiply (TaggedPolygonVectorR polygons, TransformCR transform);
//! Multiply all points by DMatrix4d and renormalize
static void MultiplyAndRenormalize (TaggedPolygonVectorR polygons, DMatrix4dCR matrix);

//! Shuffle arrays so that outer loops precede their immediately contained inner loops.
//! @param [in,out] loops polygons to sort.
//! @param [in] planeNormal normal to the plane in which containment is viewed.
//! @param [out] lastInBlock indices of the last loop of each outer+holes block
//!       loop indices 0<=i<=lastInBlock[0] are the first block.
//!       loop indices lastInBlock[0]+1<=i<=lastInBlock[2] are the second block.
static void SortAndMarkContainment (TaggedPolygonVectorR loops, DVec3dCR planeNormal, bvector<size_t> &lastInBlock);

//! Reverse order in the array.  Does not affect contents of loops (e.g. does not reverse direction of individual polygons)
static void Reverse (TaggedPolygonVectorR loops);

//! Reverse the pointer order within each polygon
static void ReverseEachPolygon (TaggedPolygonVectorR loops);

//! Append all polygons from source, setting indexA from parameter and indexB as index in source.
static void AppendWithParentIndices (TaggedPolygonVectorR dest, TaggedPolygonVectorCR source, size_t indexA);

//! Append polygons from source, taking only those within an xy range and setting indexA from parameter and indexB as index in source.
static void AppendWithParentIndices (TaggedPolygonVectorR dest, TaggedPolygonVectorCR source, size_t indexA, DRange3dCR xySelectRange);


//! Search for polygons that are non-planar.
//! Triangulate nonplanar polygons.
//! When a polygon is triangulated, one triangle replaces it, others go at end.
static void TriangulateNonPlanarPolygons (TaggedPolygonVectorR polygons, double abstol);

static void CutAndFill
(
TaggedPolygonVectorCR source,
TaggedPolygonVectorR surfaceAaboveB,
TaggedPolygonVectorR surfaceAbelowB,
TaggedPolygonVectorR surfaceBaboveA,
TaggedPolygonVectorR surfaceBbelowA,
bvector<TaggedPolygonVector> &debugShapes
);

static void CutAndFill_IndexedHeap
(
TaggedPolygonVectorCR source,
TaggedPolygonVectorR surfaceAaboveB,
TaggedPolygonVectorR surfaceAbelowB,
TaggedPolygonVectorR surfaceBaboveA,
TaggedPolygonVectorR surfaceBbelowA,
bvector<TaggedPolygonVector> &debugShapes
);

static void Punch
(
TaggedPolygonVectorCR targets,
TaggedPolygonVectorCR cutters,
bool keepInside,
TaggedPolygonVectorR  output,
bvector<TaggedPolygonVector> &debugShapes
);

static void PunchByPlaneSets
(
PolyfaceQueryCR mesh,
TransformCR meshToPunch,       // applied to mesh only !!
TaggedPolygonVectorCR punchPolygons,
TaggedPolygonVector* insidePolygons,
TaggedPolygonVector* outsidePolygons,
TaggedPolygonVector* debugPolygons
);

//! Reverse all faces for which areaXY * factor < 0.
void ReverseForAreaXYSign (TaggedPolygonVector &polygons, double factor = 1.0);
};




//
//
// Interface for clipping a base polygon with many clippers.
// A single clipper may be used for multiple setup-and-clip sequences.
// The clipee may be a set of multiple polygons, or a polygon with holes.
//
// PolygonMarkedMultiClipP clipper = VuMultiClipFactory::NewPolygonMarkedMultiClip ();
// for each clippee
//      {
//      // Send the base polygon, then the clippers ....
//      clipper->SetBasePolyogn ();
//      for each clip polygon
//           clipper->AddClipPolygon ();
//      clipper->FinishClip ();      --- all the work happens here.
//      // Visit faces of the result.
//      for (clipper->SetupForLoopOverFaces (); clipper->GetFace (...);))
//          ... process face
//      }
// VuMultiClipFactory::FreePolygonMarkedMultiClip (clipper);
//
class GEOMDLLIMPEXP PolygonMarkedMultiClip
{
public:

enum FaceMode
    {
    FaceMode_Default,
    FaceMode_Triangulate,
    FaceMode_Convex
    };

GEOMAPI_VIRTUAL ~PolygonMarkedMultiClip() {;}

//! Initialize for new clipee and clip sequence.
//! Reusing a PolygonMarkedMultiClip for multiple data sets is strongly recommended --
//!    it eliminates repeated heap allocation and release
GEOMAPI_VIRTUAL void Initialize () = 0;

//! Clear the clipper.  Set up the base polygon.  Optionally put a mark bit on selected edges.
GEOMAPI_VIRTUAL bool AddBasePolygon (DPoint3d *pXYZArray, bool *pMarkBits, int numXYZ) = 0;

//! Add a clip polygon.  Optionally put a mark bit on selected edges.
GEOMAPI_VIRTUAL bool AddClipper (DPoint3d *pXYZArray, bool *pMarkBits, int numXYZ) = 0;

//! Merge all the clippers together and subtract from the base polygon.
GEOMAPI_VIRTUAL void FinishClip (FaceMode faceMode = FaceMode_Default) = 0;

//! Setup to iterate over all faces, applying an in/out test on 
//! @param [in] primarySelect true (resp false) to collect faces that are INSIDE (resp OUTSIDE) the primary polygon
//! @param [in] clipperSelect true (resp false) to collect faces t are INSIDE (resp OUTSIDE) the secondary polygon
GEOMAPI_VIRTUAL void SetupForLoopOverFaces (bool primarySelect, bool clipperSelect) = 0;

//! Get xyz of one face.  return false if no more faces ....
//! @param [out] pXYZBuffer buffer to receive points.
//! @param [out] pMarkBits optional buffer for marker bits
//! @param [out] numXYZ number of returned points.
//! @param [in] maxXYZ buffer size.   No more points will be put in buffer,
//!         but there is no error indicator if points were skipped.
//! @param [in] repeatFirstPoint true to force extra point as wraparound to start point.
GEOMAPI_VIRTUAL bool GetFace (DPoint3d *pXYZBuffer, bool *pMarkBits,
        int &numXYZ, int maxXYZ, bool repeatFirstPoint = true) = 0;

//! Set debug level.
//!  debug > 0 prints final graph.
//!  debug > 3 prints intermediate graphs.
//!  (Console output might not work under microstation?)
GEOMAPI_VIRTUAL void SetDebug (int debug) = 0;
};

//
// Factory class to create and destroy PolygonMarkedMultiClip instances....
//
class GEOMDLLIMPEXP VuMultiClipFactory
{
private:
    VuMultiClipFactory ();  // Not instantiable.
public:
static PolygonMarkedMultiClipP NewPolygonMarkedMultiClip ();
static void FreePolygonMarkedMultiClip (PolygonMarkedMultiClipP);
};

// Interface for inquiries about clip priority.
struct XYVisibilityClipQuery
{
struct ClipSelections
    {
    bool useBelow;
    bool useOn;
    bool useAbove;
    ClipSelections (bool _useBelow, bool _useOn, bool _useAbove);
    };
//! Ask how to source[clipperIndex] clips source[clippeeIndex]
//! @param [out] selections  returned actions for clipper portions below, on, and above.
GEOMAPI_VIRTUAL ClipSelections GetClipAction
(
TaggedPolygonVectorCR source,
size_t  clippeeIndex,
size_t  clipperIndex
) = 0;

};

//! Analyze xy visibility of all polygons in source.   Return all visible parts in dest.
//! Each source polygons is considered as a primary (clippee) polygon A.
//! For each primary, all others are considered as clippers.  The union of all clippers
//!    is treated a the secondary region B.
//! The returned polygons satsify (inA == selectA) AND (inB == selectB)
//! @param source input polygons
//! @param dest output polygons
//! @param select1 true to return parts of source that are inside the primary polygon (A)
//! @param seelct2 true to return parts of source that outside the clipper
//! @remark This is equivalent to the longer parameter list with a GetClipAction object
//!             that returns (false, clipperIndex < clippeeIndex, true)
GEOMDLLIMPEXP void bsiPolygon_clipByXYVisibility (TaggedPolygonVectorCR source, TaggedPolygonVectorR dest,
            bool selectA = true, bool selectB = true);

//!
GEOMDLLIMPEXP void bsiPolygon_clipByXYVisibility
(
TaggedPolygonVectorCR source,
TaggedPolygonVectorR dest,
bool select1,
bool select2,
XYVisibilityClipQuery &actionSelector
);

//! Analyze xy visibility of a single polygon when clipped by all polygons in source.   Return all visible parts in dest.
//! Each source polygon is considered as a clipper.  The union of all clippers
//!    is treated as the secondary region B.
//! The returned polygons satsify (inA == selectA) AND (inB == selectB)
//! @param polygon primary polygon.
//! @param clippers clipping polygons
//! @param dest output polygons
//! @param selectBelow true to return primary polygon portions that are below all clippers
//! @param selctOn true to return primary polygon parts when they are on the highest clipper
//! @param selectAbove tru to return primary polygon portions that are above all clippers.
GEOMDLLIMPEXP void bsiPolygon_clipOnePolygonByXYVisibility
(
TaggedPolygonCR polygon,
TaggedPolygonVectorCR clippers,
TaggedPolygonVectorR dest,
bool selectBelow = true,
bool selectOn    = false,
bool selectAbove = true
);


//! context for (repeated) merging of shards with (a) simple edge sharing and (b) possible mid-edge artificial nodes.
//! Each call to HealShards builds up VU graph, fusses away interior edges (being careful to retain bridges for wraparound faces), and returns the composite faces.
struct ShardHealer
{
private:
VuSetP m_graph;
bvector<int> m_loopIndex;  // work vector
bvector<DPoint3d> m_loopXYZ;
double m_vertexTolerance;
bvector<size_t> m_faceClusters;
bvector<DPoint3d> m_originalXYZ;
public:
GEOMDLLIMPEXP ShardHealer ();
GEOMDLLIMPEXP ~ShardHealer ();

private:
void SetOriginalXYZ (bvector<DPoint3d> const &originalXYZ);
// linear search for point in data array.
int FindOriginalIndex (DPoint3dCR xyz);

// find null faces whose mates are marked exterior.
// swap at both ends to make the exterior masks be on the null face
bool SwapExteriorMasksToNullFaces (VuMask exteriorMask);
// set or clear mask at successor of nodeA:
// set if there is a sharp turn or it is an original vertex
// clear otherwise (i.e. it is a split of an original edge
void AnnotateSuccessorIfTurnOrOriginal (VuP nodeA, VuMask mask);
bool SetupInteriorFaceNumbers (VuMask exteriorMask);
// find null face pairs that separate faces. mark with merge mask if the faces can be safely merged.
// return the merge count.
size_t MergeFacesAcrossNullFaces
(
VuMask exteriorMask,
VuMask mergeMask        // apply to all 4 edges of deletable pair
);


public:
GEOMDLLIMPEXP bool HealShards (BVectorCache<DPoint3d> &shards, bvector<DPoint3d> &originalXYZ);
};

END_BENTLEY_GEOMETRY_NAMESPACE
