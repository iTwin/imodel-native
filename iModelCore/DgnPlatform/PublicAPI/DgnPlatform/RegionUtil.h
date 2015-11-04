/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/RegionUtil.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>

//__PUBLISH_SECTION_END__
#include <DgnPlatform/NullContext.h>
#include <DgnPlatform/SimplifyViewDrawGeom.h>

#include <Regions/regionsAPI.h>
#include <Regions/rimsbsAPI.h>
//__PUBLISH_SECTION_START__

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

//__PUBLISH_SECTION_END__
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

//__PUBLISH_SECTION_START__
/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  09/09
+===============+===============+===============+===============+===============+======*/
struct RegionParams
    {
    private:

    RegionType      m_type;
    RegionLoops     m_regionLoops;

    bool            m_associative;
    bool            m_invisibleBoundary;
    bool            m_interiorText;
    bool            m_forcePlanar;
    bool            m_dirty;

    uint32_t        m_reservedFlags;

    double          m_gapTolerance;
    double          m_textMarginFactor;

    RotMatrix       m_flatten;

    public:

    DGNPLATFORM_EXPORT                  RegionParams ();

    DGNPLATFORM_EXPORT void             SetType (RegionType regionType);
    DGNPLATFORM_EXPORT void             SetFloodParams (RegionLoops regionLoops, double gapTolerance);
    DGNPLATFORM_EXPORT void             SetInteriorText (bool interiorText, double textMarginFactor);
    DGNPLATFORM_EXPORT void             SetAssociative (bool yesNo);
    DGNPLATFORM_EXPORT void             SetInvisibleBoundary (bool yesNo);
    DGNPLATFORM_EXPORT void             SetFlattenBoundary (bool yesNo, RotMatrixCP flatten);
    DGNPLATFORM_EXPORT void             SetDirty (bool yesNo);

    DGNPLATFORM_EXPORT RegionType       GetType () const;
    DGNPLATFORM_EXPORT RegionLoops      GetFloodParams (double* gapTolerance) const;
    DGNPLATFORM_EXPORT bool             GetInteriorText (double* textMarginFactor) const;
    DGNPLATFORM_EXPORT bool             GetAssociative () const;
    DGNPLATFORM_EXPORT bool             GetInvisibleBoundary () const;
    DGNPLATFORM_EXPORT bool             GetFlattenBoundary (RotMatrixP flatten) const;
    DGNPLATFORM_EXPORT bool             GetDirty () const;
    }; // RegionParams

//__PUBLISH_SECTION_END__
/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  09/09
+===============+===============+===============+===============+===============+======*/
struct RegionGraphicsDrawGeom : SimplifyViewDrawGeom
{
    DEFINE_T_SUPER(SimplifyViewDrawGeom)
private:
    RG_Header*          m_pRG;
    RIMSBS_Context*     m_pCurves;
    MTG_MarkSet         m_activeFaces;
    int                 m_currentGeomMarkerId;
    double              m_textMarginFactor;
    bool                m_interiorText;
    // clang says not used - bool m_isAssociative;
    bool                m_isFlood;

    bool                m_forcePlanar;
    Transform           m_flattenTrans;
    DVec3d              m_flattenDir;

    RegionErrors        m_regionError;
    CurveVectorPtr      m_textBoundaries; // Union region collected from draw

    protected:

    GeometricElementCP  GetCurrentElement();
    bool                ComputePostFlattenTransform (CurveVectorCR region);
    void                ResetPostFlattenTransform ();
    virtual void        _SetDrawViewFlags (ViewFlags flags) override;
    virtual bool        _ClipPreservesRegions () const override {return false;} // Want fast open curve clip...
    virtual bool        _DoClipping () const override {return m_context->IsAttached ();} // Only want for initial flood create...
    virtual bool        _DoTextGeometry () const override {return false;}
    virtual bool        _DoSymbolGeometry () const override {return false;}
    virtual StatusInt   _ProcessCurvePrimitive (ICurvePrimitiveCR, bool closed, bool filled) override;
    virtual StatusInt   _ProcessCurveVector (CurveVectorCR, bool filled) override;
    virtual StatusInt   _ProcessSolidPrimitive (ISolidPrimitiveCR) override {return SUCCESS;}
    virtual StatusInt   _ProcessSurface (MSBsplineSurfaceCR surface) override {return SUCCESS;}
    virtual StatusInt   _ProcessBody (ISolidKernelEntityCR entity) override {return SUCCESS;}
    virtual StatusInt   _ProcessFacetSet (PolyfaceQueryCR facets, bool isFilled) override {return SUCCESS;}

    virtual void        _DrawTextString (TextStringCR text, double* zDepth) override;

public:

DGNPLATFORM_EXPORT  RegionGraphicsDrawGeom ();
DGNPLATFORM_EXPORT  ~RegionGraphicsDrawGeom ();

int                 GetCurrentGeomMarkerId ();

void                SetTextMarginFactor (bool interiorText, double textMarginFactor) {m_interiorText = interiorText; m_textMarginFactor = textMarginFactor;}
double              GetTextMarginFactor () {return m_textMarginFactor;}
bool                GetInteriorText () {return m_interiorText;}

void                SetFlattenBoundary (TransformCR flattenTrans) {m_forcePlanar = true; m_flattenTrans = flattenTrans;}
void                SetFlattenBoundary (DVec3dCR flattenDir) {m_forcePlanar = true; m_flattenDir = flattenDir;}
TransformCP         GetFlattenBoundary () {return m_forcePlanar ? &m_flattenTrans : NULL;}

void                SetIsFlood (bool isFlood) {m_isFlood = isFlood;}
BentleyStatus       SetupGraph (double gapTolerance, bool mergeHoles);
void                SetAbortFunction (RGC_AbortFunction abort);
RegionErrors        GetRegionError () {return m_regionError;}

BentleyStatus       CollectBooleanFaces (RGBoolSelect boolOp, int highestOperandA, int highestOperandB);
void                CollectFaceLoopsAtPoint (bvector<MTGNodeId>* faceNodeIds, DPoint3dCR seedPoint, RegionLoops floodSelect, bool stepOutOfHoles);
void                CollectByInwardParitySearch (bool parityWithinComponent, bool vertexContactSufficient);

void                AddFaceLoop (MTGNodeId faceNodeId);
void                RemoveFaceLoop (MTGNodeId faceNodeId);
bool                ToggleFaceLoop (MTGNodeId faceNodeId);
bool                IsFaceLoopSelected (MTGNodeId faceNodeId);

void                GetFaceLoops (CurveVectorPtr& region, bvector<MTGNodeId>& faceNodeIds);
BentleyStatus       GetActiveRegions (CurveVectorPtr& region);
BentleyStatus       GetMarkedRegions (CurveVectorPtr& region, MTG_MarkSet* markSet);

BentleyStatus       GetRoots (bvector<DgnElementId>& regionRoots);               // No duplicates...
BentleyStatus       GetRoots (bvector<DgnElementId>&, CurveVectorCR region);      // May contain duplicates...
BentleyStatus       GetRoots (bvector<DgnElementId>&, ICurvePrimitiveCR curve);   // May contain duplicates...

}; // RegionGraphicsDrawGeom

struct FloodSeed
    {
    DPoint3d            m_pt;
    bvector<MTGNodeId>  m_faceNodeIds;
    };

//__PUBLISH_SECTION_START__
typedef RefCountedPtr<RegionGraphicsContext> RegionGraphicsContextPtr;

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  09/09
+===============+===============+===============+===============+===============+======*/
struct RegionGraphicsContext : RefCountedBase
                               //__PUBLISH_SECTION_END__
                               ,NullContext
                               //__PUBLISH_SECTION_START__
{
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(NullContext)
protected:

RegionGraphicsDrawGeom  m_output;

RegionType              m_operation;
RegionLoops             m_regionLoops;
double                  m_gapTolerance;
bool                    m_stepOutOfHoles;
bool                    m_setLoopSymbology;
bool                    m_updateAssocRegion;
bool                    m_cullRedundantLoop;

DgnModelP               m_targetModel;
bvector<FloodSeed>      m_floodSeeds;
FloodSeed               m_dynamicFaceSeed;

DgnModelP GetViewTarget () {return m_targetModel;}

virtual void            _DrawAreaPattern (ClipStencil& boundary) override {}
virtual void            _DrawTextString (TextStringCR text) override;
virtual ILineStyleCP    _GetCurrLineStyle (Render::LineStyleSymbP* symb) override {return nullptr;}

BentleyStatus           PushBooleanCandidate (GeometricElementCR element, TransformCP trans);
BentleyStatus           SetTargetModel (DgnModelR targetModel);
BentleyStatus           VisitFloodCandidate (GeometricElementCR element, TransformCP trans);
BentleyStatus           VisitBooleanCandidate (GeometricElementCR element, TransformCP trans, bvector<DMatrix4d>* wireProducts = NULL, bool allowText = false);

BentleyStatus           CreateRegionElement (DgnElementPtr& elm, CurveVectorCR region, bvector<DgnElementId> const* regionRoots, bool is3d);
BentleyStatus           CreateRegionElements (DgnElementPtrVec& out, CurveVectorCR region, bvector<DgnElementId> const* regionRoots, bool is3d);

public:

DGNPLATFORM_EXPORT               RegionGraphicsContext ();

                   bool          IsGraphInitialized () {return NULL != GetViewTarget();}
                   RegionErrors  GetRegionError () {return m_output.GetRegionError ();}
DGNPLATFORM_EXPORT void          SetAbortFunction (RGC_AbortFunction abort);
                   void          SetAssociativeRegionUpdate () {m_updateAssocRegion = true;}
                   void          SetCullRedundantLoops () {m_cullRedundantLoop = true;}

DGNPLATFORM_EXPORT BentleyStatus PopulateGraph (DgnViewportP vp, DgnElementCPtrVec const* in);
DGNPLATFORM_EXPORT BentleyStatus PopulateGraph (DgnModelR targetModel, DgnElementCPtrVec const& in, TransformCP inTrans);
DGNPLATFORM_EXPORT BentleyStatus AddFaceLoopsAtPoints (DPoint3dCP seedPoints, size_t numSeed);
DGNPLATFORM_EXPORT void          AddFaceLoopsByInwardParitySearch (bool parityWithinComponent, bool vertexContactSufficient) {m_output.CollectByInwardParitySearch (parityWithinComponent, vertexContactSufficient);}
DGNPLATFORM_EXPORT bool          ToggleFaceAtPoint (DPoint3dCR seedPoint);
DGNPLATFORM_EXPORT bool          GetFaceAtPoint (CurveVectorPtr& region, DPoint3dCR seedPoint);
DGNPLATFORM_EXPORT int           GetCurrentFaceNodeId (); // NOTE: Valid after calling GetFaceAtPoint...
DGNPLATFORM_EXPORT bool          GetActiveFaces (CurveVectorPtr& region);
DGNPLATFORM_EXPORT bool          IsFaceAtPointSelected (DPoint3dCR seedPoint);

DGNPLATFORM_EXPORT BentleyStatus GetRoots (bvector<DgnElementId>& regionRoots) {return m_output.GetRoots (regionRoots);}                                 // No duplicates...
DGNPLATFORM_EXPORT BentleyStatus GetRoots (bvector<DgnElementId>& regionRoots, CurveVectorCR region) {return m_output.GetRoots (regionRoots, region);}   // May contain duplicates...
DGNPLATFORM_EXPORT BentleyStatus GetRoots (bvector<DgnElementId>& regionRoots, ICurvePrimitiveCR curve) {return m_output.GetRoots (regionRoots, curve);} // May contain duplicates...

void                             EnableOriginalLoopSymbology () {m_setLoopSymbology = true;} // Legacy behavior of create grouped hole tool...

DGNPLATFORM_EXPORT bool          GetAdjustedSeedPoints (bvector<DPoint3d>* seedPoints);
DGNPLATFORM_EXPORT BentleyStatus UpdateAssociativeRegion (DgnElementPtr& elm);
DGNPLATFORM_EXPORT BentleyStatus BooleanWithHoles (DgnModelR targetModel, DgnElementCPtrVec const& in, DgnElementCPtrVec const& holes, TransformCP inTrans, TransformCP holeTrans, RegionType operation);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

//! Set flood parameters for boundary gap tolerance and finding interior holes.
DGNPLATFORM_EXPORT void SetFloodParams (RegionLoops regionLoops, double gapTolerance, bool stepOutOfHoles = false);

//! Set flood parameters for treating text as holes.
DGNPLATFORM_EXPORT void SetInteriorText (bool interiorText, double textMarginFactor);

//! Set a flatten transform for producing a planar region from non-planar boundaries.
DGNPLATFORM_EXPORT void SetFlattenBoundary (TransformCR flattenTrans);

//! Set a flatten direction for producing a planar region from non-planar boundaries.
DGNPLATFORM_EXPORT void SetFlattenBoundary (DVec3dCR flattenDir);

//! Find closed regions from supplied boundary candidates using flood parameters and seed point locations.
//! @note inTrans is an array pf size in.GetCount of tranforms for each boundary candidate, can be NULL if all boundaries are in the coordinates of the targetModel.
DGNPLATFORM_EXPORT BentleyStatus Flood (DgnModelR targetModel, DgnElementCPtrVec const& in, TransformCP inTrans, DPoint3dCP seedPoints, size_t numSeed);

//! Create closed regions by boolean of curve vectors.
DGNPLATFORM_EXPORT BentleyStatus Boolean (DgnModelR targetModel, bvector<CurveVectorPtr> const& in, RegionType operation);

//! Create closed regions by boolean of closed boundary candidates.
//! @note inTrans is an array pf size in.GetCount of tranforms for each boundary candidate, can be NULL if all boundaries are in the coordinates of the targetModel.
//! @note An associative region element can be created for a single closed boundary element using a RegionType of RegionType::ExclusiveOr.
DGNPLATFORM_EXPORT BentleyStatus Boolean (DgnModelR targetModel, DgnElementCPtrVec const& in, TransformCP inTrans, RegionType operation);

//! Create closed regions by boolean between separate target and tool boundary candidates agendas.
//! @note inTrans is an array pf size in.GetCount of tranforms for each boundary candidate, can be NULL if all boundaries are in the coordinates of the targetModel.
DGNPLATFORM_EXPORT BentleyStatus Boolean (DgnModelR targetModel, DgnElementCPtrVec const& target, DgnElementCPtrVec const& tool, TransformCP targetTrans, TransformCP toolTrans, RegionType operation);

//! Initialize associative region element parameters from current context settings for use with GetAssociativeRegion.
DGNPLATFORM_EXPORT void InitRegionParams (RegionParams& params);

//! Return region result as a CurveVector that represents a closed path, parity region, or union region.
DGNPLATFORM_EXPORT BentleyStatus GetRegion (CurveVectorPtr& region);

//! Return region result as a single element that represents a closed path, parity region, or union region.
DGNPLATFORM_EXPORT BentleyStatus GetRegion (DgnElementPtr& elm);

//! Return region result as an agenda of closed elements and grouped holes.
DGNPLATFORM_EXPORT BentleyStatus GetRegions (DgnElementPtrVec& out);

//! Return region result as an associative region element.
//! @note The supplied boundary candidates must be persistent elements or an associative region can't be created.
DGNPLATFORM_EXPORT BentleyStatus GetAssociativeRegion (DgnElementPtr& elm, RegionParams const& params, WCharCP cellName);

//! Create an instance of an RegionGraphicsContext for the purpose of creating closed regions by flood or boolean operation.
//! @return A reference counted pointer to a RegionGraphicsContext.
DGNPLATFORM_EXPORT static RegionGraphicsContextPtr Create ();

}; // RegionGraphicsContext

END_BENTLEY_DGN_NAMESPACE

