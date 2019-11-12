/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/NullContext.h>
#include <DgnPlatform/SimplifyGraphic.h>

BEGIN_BENTLEY_DGN_NAMESPACE

enum class RegionLoops
    {
    Ignore          = 0, //! Don't look for interior loops
    Outer           = 1, //! Only consider outermost interior loops
    Alternating     = 2, //! Use parity rules and find all interior loops
    };

enum class RegionType
    {
    Flood           = 0, //! Region created by searching for closed loops around seed points
    Union           = 1, //! Region created by union of closed curves
    Intersection    = 2, //! Region created by intersection of closed curves
    Difference      = 3, //! Region created by difference of closed curves
    ExclusiveOr     = 4, //! Region created by using parity rules for hole loops
    };

enum RegionErrors
    {
    REGION_ERROR_None           = SUCCESS,
    REGION_ERROR_NonCoplanar    = (-100),
    };

enum RegionLoopOEDCode
    {
    DWG_OUTERMOST_PATH          = 1,
    DWG_EXTERNAL_PATH           = 2,
    DWG_DERIVED_PATH            = 3,
    DWG_TEXTBOX_PATH            = 4,
    };

enum RegionConstants
    {
    MAX_FloodSeedPoints         = 100, //! Limit on the number of seed point in a flood type region
    };

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  09/09
+===============+===============+===============+===============+===============+======*/
struct RegionParams
    {
    private:

    RegionType      m_type = RegionType::ExclusiveOr;
    RegionLoops     m_regionLoops = RegionLoops::Ignore;

    bool            m_associative = false;
    bool            m_invisibleBoundary = false;
    bool            m_forcePlanar = true;
    bool            m_dirty = false;

    double          m_gapTolerance = 0.0;
    RotMatrix       m_flatten = RotMatrix::FromIdentity();

    public:

    void SetType(RegionType regionType) {m_type = regionType;}
    void SetFloodParams(RegionLoops regionLoops, double gapTolerance) {m_regionLoops = regionLoops; m_gapTolerance = gapTolerance;}
    void SetAssociative(bool yesNo) {m_associative = yesNo;}
    void SetInvisibleBoundary(bool yesNo) {m_invisibleBoundary = yesNo;}
    void SetFlattenBoundary(bool yesNo, RotMatrixCP flatten) {m_forcePlanar = yesNo; if (flatten) m_flatten = *flatten;}
    void SetDirty(bool yesNo) {m_dirty = yesNo;}
    
    RegionType GetType() const {return m_type;}
    RegionLoops GetFloodParams(double* gapTolerance) const {if (gapTolerance) *gapTolerance = m_gapTolerance; return m_regionLoops;}
    bool GetAssociative() const {return m_associative;}
    bool GetInvisibleBoundary() const {return m_invisibleBoundary;}
    bool GetFlattenBoundary(RotMatrixP flatten) const {if (flatten) *flatten = m_flatten; return m_forcePlanar;}
    bool GetDirty() const {return m_dirty;}

    }; // RegionParams

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  09/09
+===============+===============+===============+===============+===============+======*/
struct FloodSeed
    {
    DPoint3d      m_pt;
    bvector<int>  m_faceNodeIds;
    };

struct IRegionData : public IRefCounted {};
typedef RefCountedPtr<IRegionData> IRegionDataPtr;
typedef RefCountedPtr<RegionGraphicsContext> RegionGraphicsContextPtr;
typedef bool (*RegionGraphicsContext_AbortFunction) (void*);

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  09/09
+===============+===============+===============+===============+===============+======*/
struct RegionGraphicsContext : RefCountedBase, NullContext, IGeometryProcessor
{
    DEFINE_T_SUPER(NullContext)
    friend struct RegionData;

    //=======================================================================================
    //! Helps filter out geometry to be used for the region
    // @bsistruct                                                   Diego.Pinate    06/18
    //=======================================================================================
    struct IRegionFilter : RefCountedBase
        {
        //! Provides a way to filter flood regions based on elements for push-pull tool
        //! @param source [in] GeometrySource passed by the context
        //! @return true if the element needs to be filtered out and not used as a boundary, false otherwise
        virtual bool _FilterSource(GeometrySourceCR source) = 0;
        };
    typedef RefCountedPtr<IRegionFilter> IRegionFilterPtr;

protected:

IRegionDataPtr      m_regionData;
int                 m_currentGeomMarkerId = 0;
bool                m_isAssociative = false;

bool                m_forcePlanar = false;
bool                m_applyPerspective = false;
Transform           m_flattenTrans = Transform::FromIdentity();
DVec3d              m_flattenDir = DVec3d::UnitZ();

bool                m_restrictToPlane = false;
DPlane3d            m_plane = DPlane3d::FromOriginAndNormal(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

RegionErrors        m_regionError = REGION_ERROR_None;
RegionType          m_operation = RegionType::Flood;
RegionLoops         m_regionLoops = RegionLoops::Ignore;
double              m_gapTolerance = 0.0;
bool                m_stepOutOfHoles = false;
bool                m_cullRedundantLoop = false;

bvector<FloodSeed>  m_floodSeeds;
FloodSeed           m_dynamicFaceSeed;
GeometrySourceCP    m_currentGeomSource = nullptr;
IRegionFilterPtr    m_regionFilter = nullptr;

bool ComputePostFlattenTransform(CurveVectorCR region);
void ResetPostFlattenTransform();

Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override {SimplifyGraphic* graphic = new SimplifyGraphic(params, *this, *this); return graphic;}
bool _ProcessCurvePrimitive(ICurvePrimitiveCR, bool closed, bool filled, SimplifyGraphic&) override;
bool _ProcessCurveVector(CurveVectorCR, bool filled, SimplifyGraphic&) override;
StatusInt _OutputGeometry(GeometrySourceCR) override;
bool _WantAreaPatterns() override {return false;}
bool _WantLineStyles() override {return false;}

DGNPLATFORM_EXPORT RegionGraphicsContext();

void SetCullRedundantLoops() {m_cullRedundantLoop = true;}
BentleyStatus VisitBooleanCandidate(GeometrySourceCR element, bvector<DMatrix4d>* wireProducts = NULL, bool allowText = false);

DGNPLATFORM_EXPORT void AddFaceLoopsByInwardParitySearch(bool parityWithinComponent, bool vertexContactSufficient);
DGNPLATFORM_EXPORT int  GetCurrentFaceNodeId(); // NOTE: Valid after calling GetFaceAtPoint...
DGNPLATFORM_EXPORT BentleyStatus BooleanWithHoles(DgnElementCPtrVec const& in, DgnElementCPtrVec const& holes, RegionType operation);

public:

DGNPLATFORM_EXPORT bool IsGraphInitialized(); //!< @private Used by DgnRegionElementTool
DGNPLATFORM_EXPORT void SetAbortFunction(RegionGraphicsContext_AbortFunction abort); //!< @private Used by DgnRegionElementTool
DGNPLATFORM_EXPORT BentleyStatus PopulateGraph(DgnViewportP vp, DgnElementCPtrVec const* in = nullptr, DRange3dCP range = nullptr, DPlane3dCP plane = nullptr, CurveVectorCP boundaryEdges = nullptr); //!< @private Used by DgnRegionElementTool
DGNPLATFORM_EXPORT BentleyStatus PopulateGraph(DgnElementCPtrVec const& in); //!< @private Used by DgnRegionElementTool
DGNPLATFORM_EXPORT BentleyStatus AddFaceLoopsAtPoints(DPoint3dCP seedPoints, size_t numSeed); //!< @private Used by DgnRegionElementTool
DGNPLATFORM_EXPORT bool ToggleFaceAtPoint(DPoint3dCR seedPoint); //!< @private Used by DgnRegionElementTool
DGNPLATFORM_EXPORT bool GetFaceAtPoint(CurveVectorPtr& region, DPoint3dCR seedPoint); //!< @private Used by DgnRegionElementTool
DGNPLATFORM_EXPORT bool GetActiveFaces(CurveVectorPtr& region); //!< @private Used by DgnRegionElementTool
DGNPLATFORM_EXPORT bool IsFaceAtPointSelected(DPoint3dCR seedPoint); //!< @private Used by DgnRegionElementTool
DGNPLATFORM_EXPORT bool GetAdjustedSeedPoints(bvector<DPoint3d>* seedPoints); //!< @private Used by DgnRegionElementTool
DGNPLATFORM_EXPORT void SetPerspectiveFlatten(bool apply); //!< @private Used by DgnRegionElementTool
DGNPLATFORM_EXPORT bool IsPerspectiveRegion(); //!< @private Used by DgnRegionElementTool
DGNPLATFORM_EXPORT CurveVectorPtr GetFromPerspectiveRegion(CurveVectorCR region, bool isDynamic = false); //!< @private Used by DgnRegionElementTool

DGNPLATFORM_EXPORT void SetRegionFilter(IRegionFilterPtr filter) { m_regionFilter = filter; }

DGNPLATFORM_EXPORT BentleyStatus GetRoots(bvector<DgnElementId>& regionRoots); //!< @private Used by DgnRegionElementTool. Returns unique roots for all active faces...
DGNPLATFORM_EXPORT BentleyStatus GetRoots(bvector<DgnElementId>& regionRoots, CurveVectorCR region); //!< @private Used by DgnRegionElementTool. May contain duplicates...
DGNPLATFORM_EXPORT BentleyStatus GetRoots(bvector<DgnElementId>& regionRoots, ICurvePrimitiveCR curve); //!< @private Used by DgnRegionElementTool. May contain duplicates...

//! Get reason why region could not be created.
RegionErrors GetRegionError() {return m_regionError;}

//! Set flood parameters for boundary gap tolerance and finding interior holes.
DGNPLATFORM_EXPORT void SetFloodParams(RegionLoops regionLoops, double gapTolerance, bool stepOutOfHoles = false);

//! Set a flatten transform for producing a planar region from non-planar boundaries.
DGNPLATFORM_EXPORT void SetFlattenBoundary(TransformCR flattenTrans);

//! Set a flatten direction for producing a planar region from non-planar boundaries.
DGNPLATFORM_EXPORT void SetFlattenBoundary(DVec3dCR flattenDir);

//! Find closed regions from supplied boundary candidates using flood parameters and seed point locations.
DGNPLATFORM_EXPORT BentleyStatus Flood(DgnElementCPtrVec const& in, DPoint3dCP seedPoints, size_t numSeed);

//! Create closed regions by boolean of curve vectors. NOTE: CurveVector::Area methods also exist and may be more appropriate to a given use case...
DGNPLATFORM_EXPORT BentleyStatus Boolean(DgnDbR db, bvector<CurveVectorPtr> const& in, RegionType operation);

//! Create closed regions by boolean of closed boundary candidates.
DGNPLATFORM_EXPORT BentleyStatus Boolean(DgnElementCPtrVec const& in, RegionType operation);

//! Create closed regions by boolean between separate target and tool boundary candidates agendas.
DGNPLATFORM_EXPORT BentleyStatus Boolean(DgnElementCPtrVec const& target, DgnElementCPtrVec const& tool, RegionType operation);

//! Initialize associative region element parameters from current context settings.
DGNPLATFORM_EXPORT void InitRegionParams(RegionParams& params);

//! Return region result as a CurveVector that represents a closed path, parity region, or union region.
DGNPLATFORM_EXPORT BentleyStatus GetRegion(CurveVectorPtr& region);

//! Create an instance of an RegionGraphicsContext for the purpose of creating closed regions by flood or boolean operation.
//! @return A reference counted pointer to a RegionGraphicsContext.
DGNPLATFORM_EXPORT static RegionGraphicsContextPtr Create();

}; // RegionGraphicsContext

END_BENTLEY_DGN_NAMESPACE

