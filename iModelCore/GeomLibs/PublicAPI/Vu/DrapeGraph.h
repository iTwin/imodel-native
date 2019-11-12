/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
// interface for callback asking a boolean condition on a node representing a face.
struct VuTestCondition
{
GEOMAPI_VIRTUAL bool Test (VuSetP pGraph, VuP pFaceSeed, ptrdiff_t originalPolygonIndex) = 0;
};

// Plane with equational form {z = z00 + ax*x * ay*y}, accompanied by some extra data and (min,max) z range limits for evaluation.
struct PlaneData
{
private:
VuP    m_node00;
unsigned int m_mask;
double m_ax;
double m_ay;
double m_z00;
ptrdiff_t m_tag;
double m_zMin;
double m_zMax;
public:
static const unsigned int DanglingEdge = 1;
static const ptrdiff_t DefaultTag = -1;

PlaneData (VuP node00, double ax, double ay, double z00, ptrdiff_t tag, double zMin, double zMax);

void SetZLimits (double minZ, double maxZ);
// Evaluate Z at x,y, but cap the values at minZ and maxZ.
double EvaluateZ (double x, double y) const;
double EvaluateRestrictedZ (double x, double y) const;
double SumSquaredCoffs () const;
void SetMask (unsigned int mask);
void ClearMask (unsigned int mask);
bool IsMaskSet (unsigned int mask);
ptrdiff_t GetTag () const;
};

struct DrapeGraph;
typedef DrapeGraph *DrapeGraphP;
struct PlaneArray : bvector<PlaneData>
{
bool IsActiveIndex (size_t index);
double EvaluateZ (size_t index, double x, double y, double defaultZ, bool &ok);
double EvaluateRestrictedZ (size_t index, double x, double y, double defaultZ, bool &ok);
void SetMask (size_t index, unsigned int mask);
bool AddByOriginAndNormal (DPoint3dCR origin, DVec3dCR normal, ptrdiff_t tag, double zMin, double zMax, size_t &index);

// Add duplicate of current member addressed by index.
size_t AddDuplicate (size_t i);
ptrdiff_t GetTag (size_t index);
};

/*=================================================================================**//**
* @bsistruct                                                    Xiaoyong.Yang   03/2007
+===============+===============+===============+===============+===============+======*/
struct      DrapeGraph
{
private:
    PlaneArray m_indexedPlanes;
    VuSetP m_pGraph;
    size_t m_numVertical;    
    size_t m_maxPlanes;
    double m_maxZFringe; // maximum amount that z coordinates can be moved outside the original range of each facet.

public:
    static const VuMask s_PRIMARY_EDGE_MASK = VU_RULE_EDGE;     // Outside of primary polygons.
    static const VuMask s_BARRIER_EDGE_MASK = VU_SEAM_EDGE;     // Outside of target polygons
    static const VuMask s_WELD_EDGE_MASK = VU_KNOT_EDGE;        // edge where drop panels are welded to target.
    static const VuMask s_ANY_ACTIVE_EDGE_MASK  = VU_RULE_EDGE | VU_SEAM_EDGE;
    void SetMaxZShift (double dz);
DrapeGraph ();
~DrapeGraph ();

VuSetP GetGraph ();
bool AddPolygon (DPoint3dP points, int count, ptrdiff_t tag, VuMask outsideMask);
void AddPolygon (bvector<DPoint3d> &xyz, ptrdiff_t tag, VuMask outsideMask);
void AddPolygon (TaggedPolygon &polygon, ptrdiff_t tag, VuMask outsideMask);
void AddPolygons (TaggedPolygonVectorR polygons, VuMask outsideMaskA0, VuMask outsideMaskA1, double verticalShiftFraction);
DRange3d GetRange ();
// Expand the z parts of range by the fringe allowance of the DrapeGraph.
void ExpandZRange (DRange3dR range);


void ExtractIndexArrays (
    bvector<DPoint3d>& pointArray,
    bvector<int> &pointIndexArray,
    bvector<int> *colorIndexArray,
    bool triangulate,
    uint32_t          color,
    uint32_t          weight,
    int32_t           style,
    double          vertexTolerance,
    double          baseZ,
    VuMask          skipMask,    // Do NOT output these faces.
    bool            outputBase,
    bvector<bvector<DPoint3d>> *pExteriorLoops
    );


//! @param [in] lowZ lowest z
//! @param [in] highZ highest possible z, appears in output if internal errors.
//! @param [in] clusterTolerance tolerance for vertex clustering step (suggested: rangeDiagona/1e6)
void DoAnalysis (double lowZ, double ghZ, double clusterTolerance);
//! Apply mask to all faces that are completely at or below capZ
void MarkFacesBelow (double capZ, VuMask mask);
//! Apply mask to all faces that answer true to tester.Test (pGraph, pNode)
void MarkFaces (VuTestCondition &tester, VuMask mask);
private:
void SetZAll (double z);
void ClusterXY (double tolerance);
void MarkParityErrors ();
void MoveNodesToMaxVisibleZ (double z);


void UpdateActivePlanesForEdgeCrossing (VuP pOldFace, VuP pNewFace,
    bvector<int> &activePlaneIndex,
    bvector<int> &invalidPlaneIndices
    );
void DumpSearchData (VuP pOldFace, VuP pNewFace, bvector<int>&activePlaneIndex,
            VuMask entryMask);
void ApplyVisibleZInConnectedComponent (VuP pExteriorSeed, double minZ, double maxZ,
            VuMask visitMask,
            VuMask entryMask);
bool AddPlaneByOriginAndNormal (DPoint3dCR origin, DVec3dCR normal, ptrdiff_t tag, double zMin, double zMax, size_t &index);
void SetZAroundFace (VuP pFaceSeed, bvector<int> &activePlaneIndex, double defaultZ, double maxZ);

void FixSmallFacePlane (VuSetP pGraph, VuP seedNode);
void InstallPlaneIndexAndAdjustTransitionMaskAroundFace (VuP seedNode, bool fixMask, VuMask markMask, size_t planeIndex);
void AnalyzeSmallFaces (VuSetP pGraph, bool fixSliverFaces);
};


struct VuDebug
{
static DRange3d s_outputRange;

static bool IsRealSector (VuP pNode) { return vu_fsucc (vu_fsucc (pNode)) != pNode;}
static void PrintMaskChars (VuP pNode);
static void ShowVertex (VuSetP pGraph, VuP pSeed, char const * title = NULL);
static void ShowRange (char const * name, DRange3d range);
static void ShowVertices (VuSetP pGraph, char const * title);
static void ShowFace (VuSetP pGraph, VuP pFace, char const * name, bool showVertexLoops, bool linefeed = true);
static void ShowFaces (VuSetP pGraph, char const * title, bool detailed, bool forceOutput = false);
static void ShowElement (VuSetP pGraph) {}

static void MoveToNewOutputRange ();
static void SetOutputOrigin (DRange3dCR inputRange);
};



END_BENTLEY_GEOMETRY_NAMESPACE
