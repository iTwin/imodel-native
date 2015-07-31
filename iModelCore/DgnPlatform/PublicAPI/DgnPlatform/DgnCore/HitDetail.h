/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/HitDetail.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/bvector.h>
#include "ISprite.h"
#include "IViewDraw.h"
#include "IViewOutput.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
enum class SubSelectionMode
    {
    None        = 0, //! Select entire element - No sub-selection
    Part        = 1, //! Select single DgnGeomPart
    Primitive   = 2, //! Select single geometric primitive
    Segment     = 3, //! Select single curve primitive/line string segment
    };

//=======================================================================================
//!  Lower numbers are "better" (more important) Hits than ones with higher numbers.
//=======================================================================================
enum class HitPriority
{
    Highest     = 0,
    Vertex      = 300,
    Origin      = 400,
    Edge        = 400,
    TextBox     = 550,
    Interior    = 600,
};

//=======================================================================================
//!  The procedure that generated this Hit.
//=======================================================================================
enum class HitSource
{
    None          = 0,
    FromUser      = 1,
    MotionLocate  = 2,
    AccuSnap      = 3,
    TentativeSnap = 4,
    DataPoint     = 5,
    Application   = 6,
    EditAction    = 7,
    EditActionSS  = 8,
};

//=======================================================================================
//! The type of geometry that was being tested to generate this Hit. This is not
//! the ELEMENT type that generated the Hit, but rather what type of geometry within an element
//! being considered.
//=======================================================================================
enum class HitGeomType
{
    None           = 0,
    Point          = 1,
    Segment        = 2,
    Curve          = 3,
    Arc            = 4,
    Surface        = 5,
};

//=======================================================================================
//!  Hit detail source can be used to tell what display operation generated the geometry
//=======================================================================================
enum class HitDetailSource
{
    None         = 0,
    LineStyle    = 1,
    Pattern      = 1 << 1,
    Thickness    = 1 << 2,
    PointCloud   = 1 << 3,
    Sprite       = 1 << 4,
};

ENUM_IS_FLAGS(HitDetailSource)

//=======================================================================================
// @bsiclass                                                      Keith.Bentley   10/04
//=======================================================================================
struct  GeomDetail
{
private:

    ICurvePrimitivePtr      m_primitive;                // curve primitve for hit (world coordinates).
    DPoint3d                m_closePoint;               // the closest point on geometry (world coordinates).
    DVec3d                  m_normal;                   // surface hit normal (world coordinates).
    HitGeomType             m_geomType;                 // category hit geometry falls into.
    HitDetailSource         m_detailSource;             // mask of HitDetailSource values.
    HitPriority             m_hitPriority;              // Relative priority of hit.
    bool                    m_nonSnappable;             // non-snappable detail, ex. pattern or line style.
    double                  m_viewDist;                 // xy distance to hit (view coordinates).
    double                  m_viewZ;                    // z distance to hit (view coordinates).
    GeomStreamEntryId       m_geomId;                   // id of geometric primitive that generated this hit.

public:

    DGNPLATFORM_EXPORT void Init();

    DPoint3dCR              GetClosestPoint() const     {return m_closePoint;}
    DVec3dCR                GetSurfaceNormal() const    {return m_normal;}
    HitGeomType             GetGeomType() const         {return m_geomType;}
    HitDetailSource         GetDetailSource() const     {return m_detailSource;}
    HitPriority             GetLocatePriority() const   {return m_hitPriority;}
    bool                    IsSnappable() const         {return !m_nonSnappable;}
    double                  GetScreenDist() const       {return m_viewDist;}
    double                  GetZValue() const           {return m_viewZ;}

    void                    SetClosestPoint(DPoint3dCR pt)         {m_closePoint = pt;}
    void                    SetSurfaceNormal(DVec3dCR value)       {m_normal = value;}
    void                    SetGeomType(HitGeomType value)         {m_geomType = value; m_primitive = nullptr;} // NOTE: Use SetCurvePrimitive for HitGeomType::Segment/Arc/Curve.
    void                    SetDetailSource(HitDetailSource value) {m_detailSource = value;}
    void                    SetLocatePriority(HitPriority value)   {m_hitPriority = value;}
    void                    SetNonSnappable(bool yesNo)            {m_nonSnappable = yesNo;}
    void                    SetScreenDist(double value)            {m_viewDist = value;}
    void                    SetZValue(double value)                {m_viewZ = value;}

    //! @private
    GeomStreamEntryId GetGeomStreamEntryId() const {return m_geomId;}
    //! @private
    void SetGeomStreamEntryId(GeomStreamEntryId geomId) {m_geomId = geomId;}

    DGNPLATFORM_EXPORT bool     FillGPA (GPArrayR, bool singleSegment = true) const;
    DGNPLATFORM_EXPORT bool     GetArc(DEllipse3dR) const;
    DGNPLATFORM_EXPORT bool     GetSegment(DSegment3dR) const;
    DGNPLATFORM_EXPORT size_t   GetSegmentNumber() const;
    DGNPLATFORM_EXPORT double   GetSegmentParam() const;
    DGNPLATFORM_EXPORT double   GetCloseParam() const;
    DGNPLATFORM_EXPORT size_t   GetCloseVertex() const;
    DGNPLATFORM_EXPORT size_t   GetPointCount() const;
    DGNPLATFORM_EXPORT bool     IsValidSurfaceHit() const;  // Test for HitGeomType::Surface with valid normal (i.e. not a QVElem wireframe edge hit)...
    DGNPLATFORM_EXPORT bool     IsValidEdgeHit() const;     // Check hit types, HitGeomType::Segment/HitGeomType::Curve/HitGeomType::Arc (is GetCloseParam, etc. meaningful?)...

    DGNPLATFORM_EXPORT ICurvePrimitiveCP   GetCurvePrimitive() const;
    DGNPLATFORM_EXPORT CurvePrimitiveIdCP  GetCurvePrimitiveId() const;
    DGNPLATFORM_EXPORT HitGeomType         GetCurvePrimitiveType() const;
    DGNPLATFORM_EXPORT HitGeomType         GetEffectiveHitGeomType() const; // Return GetGeomType or GetCurvePrimitiveType for HitGeomType::Surface. 

    //! Sets ICurvePrimitive hit geometry and appropriate HitGeomType for the supplied primitive.
    //! @note Optional geomType can be explicity specified to override the default HitGeomType.
    //!       For example, an arc primitive with HitGeomType::Point denotes a hit on the arc center.
    DGNPLATFORM_EXPORT void SetCurvePrimitive(ICurvePrimitiveCP curve, TransformCP localToWorld = nullptr, HitGeomType geomType = HitGeomType::None);
};

//=======================================================================================
//! @ingroup HitDetailGroup
// @bsiclass                                                      KeithBentley    04/01
//=======================================================================================
struct HitDetail : RefCountedBase
{
protected:

    DgnViewportR        m_viewport;
    DgnElementId        m_elementId;
    HitSource           m_locateSource;         // Operation that generated the hit.
    DPoint3d            m_testPoint;            // the point that was used to search (world coordinates).
    ViewFlags           m_viewFlags;            // view flags in effect when hit was generated.
    GeomDetail          m_geomDetail;           // element specific hit details.
    IElemTopologyPtr    m_elemTopo;             // details about the topology of the element.
    SubSelectionMode    m_subSelectionMode;     // segment hilite/flash mode.

    virtual HitDetailType _GetHitType() const {return HitDetailType::Hit;}
    virtual void _GetInfoString(Utf8StringR descr, Utf8CP delimiter) const;
    virtual SubSelectionMode _GetSubSelectionMode() const {return m_subSelectionMode;}
    virtual void _SetSubSelectionMode(SubSelectionMode mode) {m_subSelectionMode = mode;}
    virtual DPoint3dCR _GetHitPoint() const {return m_geomDetail.GetClosestPoint();}
    virtual void _SetHitPoint(DPoint3dCR pt) {m_geomDetail.SetClosestPoint(pt);}
    virtual void _SetTestPoint(DPoint3dCR pt) {m_testPoint = pt;}
    virtual bool _IsSameHit(HitDetailCP otherHit) const;
    virtual void _DrawInVp(DgnViewportR, DgnDrawMode drawMode, DrawPurpose drawPurpose, bool* stopFlag) const;
    virtual void _SetHilited(DgnElement::Hilited) const;

public:
#if !defined (DOCUMENTATION_GENERATOR)
    DGNPLATFORM_EXPORT HitDetail(DgnViewportR, GeometricElementCP, DPoint3dCR testPoint, HitSource, ViewFlagsCR, GeomDetailCR);
    DGNPLATFORM_EXPORT explicit HitDetail(HitDetailCR from);
    DGNPLATFORM_EXPORT virtual ~HitDetail();

    void SetLocateSource(HitSource source) {m_locateSource = source;}
    void SetHitPoint(DPoint3dCR pt) {_SetHitPoint(pt);}
    void SetTestPoint(DPoint3dCR pt) {_SetTestPoint(pt);}
    void SetHilited(DgnElement::Hilited state) const {_SetHilited(state);}
    void SetSubSelectionMode(SubSelectionMode mode) {_SetSubSelectionMode(mode);}

    void DrawInVp(DgnViewportR vp, DgnDrawMode drawMode, DrawPurpose drawPurpose, bool* stopFlag) const {_DrawInVp(vp, drawMode, drawPurpose, stopFlag);}
    DGNPLATFORM_EXPORT bool ShouldFlashCurveSegment(ViewContextR) const; //! Check for segment flash mode before calling FlashCurveSegment.
    DGNPLATFORM_EXPORT void FlashCurveSegment(ViewContextR) const; //! Setup context.GetCurrentDisplayParams() before calling!
    
    DGNVIEW_EXPORT void DrawInView(IndexedViewportR, DgnDrawMode drawMode, DrawPurpose drawPurpose) const;
    DGNVIEW_EXPORT void DrawInAllViews(IndexedViewSetR, DgnDrawMode drawMode, DrawPurpose drawPurpose) const;
    DGNVIEW_EXPORT void Hilite(IndexedViewSetR, bool onOff) const;

    DGNPLATFORM_EXPORT void GetInfoString(Utf8StringR descr, Utf8CP delimiter) const;
    DGNPLATFORM_EXPORT DgnElement::Hilited IsHilited() const;
    DGNPLATFORM_EXPORT bool IsInSelectionSet() const;
#endif

    DGNPLATFORM_EXPORT GeometricElementCPtr GetElement() const;
    DGNPLATFORM_EXPORT DgnElementId         GetElementId() const;
    DGNPLATFORM_EXPORT DgnModelR            GetDgnModel() const;
    DGNPLATFORM_EXPORT DgnDbR               GetDgnDb() const;
    DGNPLATFORM_EXPORT DgnViewportR         GetViewport() const;
    DGNPLATFORM_EXPORT HitSource            GetLocateSource() const;
    DGNPLATFORM_EXPORT DPoint3dCR           GetTestPoint() const;

    DPoint3dCR GetHitPoint() const {return _GetHitPoint();}
    HitDetailType GetHitType() const {return _GetHitType();}
    SubSelectionMode GetSubSelectionMode() const {return _GetSubSelectionMode(); }
    bool IsSameHit(HitDetailCP otherHit) const {return _IsSameHit(otherHit);}

    DGNPLATFORM_EXPORT GeomDetailCR    GetGeomDetail() const;
    DGNPLATFORM_EXPORT GeomDetailR     GetGeomDetailW();
    DGNPLATFORM_EXPORT ViewFlagsCR     GetViewFlags() const;
    DGNPLATFORM_EXPORT IElemTopologyCP GetElemTopology() const;
    DGNPLATFORM_EXPORT void            SetElemTopology(IElemTopologyP topo);

}; // HitDetail

typedef RefCountedPtr<HitDetail> HitDetailPtr;
typedef RefCountedCPtr<HitDetail> HitDetailCPtr;
typedef bvector<RefCountedPtr<HitDetail>> HitDetailArray;

//=======================================================================================
//! The result of a "locate" is a sorted list of objects that
//! satisfied the search  criteria (a HitList). Earlier hits in the list
//! are somehow "better" than those later on.
// @bsiclass                                                      KeithBentley    04/01
//=======================================================================================
struct  HitList : private HitDetailArray
{
    // NOTE: use private inheritance -- we don't want anybody calling the base class "empty" method!
    int             m_currHit;
public:
    DGNPLATFORM_EXPORT HitList();
    DGNPLATFORM_EXPORT virtual ~HitList();

    DGNPLATFORM_EXPORT void RemoveHit(int hitNum);
    DGNPLATFORM_EXPORT void RemoveCurrentHit();
    DGNPLATFORM_EXPORT HitDetailP GetHit(int hitNum) const;
    DGNPLATFORM_EXPORT int Compare(HitDetailCP oHit1, HitDetailCP oHit2, bool comparePriority, bool compareZ) const;
    DGNPLATFORM_EXPORT void SetCurrentHit(HitDetailCP);
    DGNPLATFORM_EXPORT int AddHit(HitDetailP, bool allowDuplicates, bool comparePriority);
    DGNPLATFORM_EXPORT void Empty();
    DGNPLATFORM_EXPORT void ResetCurrentHit() {m_currHit = -1;}
    DGNPLATFORM_EXPORT HitDetailP GetCurrentHit() const;
    DGNPLATFORM_EXPORT HitDetailP GetNextHit();
    DGNPLATFORM_EXPORT bool RemoveHitsFrom(HitDetailCR hit);
    DGNPLATFORM_EXPORT bool RemoveHitsFrom(DgnElementCR element);
    DGNPLATFORM_EXPORT bool RemoveHitsFrom(DgnModelR modelRef);
    DGNPLATFORM_EXPORT virtual void Dump(WCharCP label) const;

    // Because we use private inheritance, we must re-export every HitDetailArray method that we want to expose.
    DGNPLATFORM_EXPORT int GetCount() const;
    DGNPLATFORM_EXPORT HitDetailP Get(int i);
    DGNPLATFORM_EXPORT void Set(int i, HitDetailP);
    DGNPLATFORM_EXPORT void Insert(int i, HitDetailP);
    DGNPLATFORM_EXPORT void DropNulls();
};

//=======================================================================================
// @bsiclass                                                      KeithBentley    04/01
//=======================================================================================
enum KeypointType
{
    KEYPOINT_TYPE_Nearest         = 0,
    KEYPOINT_TYPE_Keypoint        = 1,
    KEYPOINT_TYPE_Midpoint        = 2,
    KEYPOINT_TYPE_Center          = 3,
    KEYPOINT_TYPE_Origin          = 4,
    KEYPOINT_TYPE_Bisector        = 5,
    KEYPOINT_TYPE_Intersection    = 6,
    KEYPOINT_TYPE_Tangent         = 7,
    KEYPOINT_TYPE_Tangentpoint    = 8,
    KEYPOINT_TYPE_Perpendicular   = 9,
    KEYPOINT_TYPE_Perpendicularpt = 10,
    KEYPOINT_TYPE_Parallel        = 11,
    KEYPOINT_TYPE_Point           = 12,
    KEYPOINT_TYPE_PointOn         = 13,
    KEYPOINT_TYPE_Unknown         = 14,
    KEYPOINT_TYPE_Custom          = 15,
};

//=======================================================================================
// @bsiclass                                                      Keith.Bentley   10/04
//=======================================================================================
enum SnapHeat
{
    SNAP_HEAT_None       = 0,
    SNAP_HEAT_NotInRange = 1,   // "of interest", but out of range
    SNAP_HEAT_InRange    = 2,
};

//=======================================================================================
// @bsiclass                                                      KeithBentley    04/01
//=======================================================================================
struct  SnapDetail : HitDetail
{
    DEFINE_T_SUPER(HitDetail)

protected:
    SnapHeat            m_heat;
    Point2d             m_screenPt;
    int                 m_divisor;
    ISpriteP            m_sprite;
    SnapMode            m_snapMode;         // snap mode currently associated with this snap
    SnapMode            m_originalSnapMode; // snap mode used when snap was created, before constraint override was applied
    double              m_minScreenDist;    // minimum distance to element in screen coordinates.
    DPoint3d            m_snapPoint;        // hitpoint adjusted by snap
    DPoint3d            m_adjustedPt;       // sometimes accusnap adjusts the point after the snap.
    int                 m_customKeypointSize;
    Byte*               m_customKeypointData;
    bool                m_allowAssociations;

    DGNPLATFORM_EXPORT virtual DPoint3dCR _GetHitPoint() const override;
    DGNPLATFORM_EXPORT virtual void _SetHitPoint(DPoint3dCR snapPt) override;
    virtual HitDetailType _GetHitType() const override {return HitDetailType::Snap;}
    virtual SnapDetail* _Clone() const;

public:
    DGNPLATFORM_EXPORT explicit SnapDetail(HitDetailCP from);
    DGNPLATFORM_EXPORT explicit SnapDetail(SnapDetail const&);
    DGNPLATFORM_EXPORT ~SnapDetail();

    void SetScreenPoint(Point2d const& pt) {m_screenPt = pt;} //!< Always set by DgnPlatform snap logic
    
    bool GetAllowAssociations() const {return m_allowAssociations;}
    void SetAllowAssociations(bool allowAssociations) {m_allowAssociations = allowAssociations;}

    ISpriteP GetSprite() const {return m_sprite;}
    void SetSprite(ISpriteP sprite) {if (m_sprite) m_sprite->Release(); m_sprite = sprite; if (m_sprite) m_sprite->AddRef();}

    DGNPLATFORM_EXPORT bool GetCustomKeypoint(int* nBytesP, Byte** dataPP) const {if (nBytesP) *nBytesP = m_customKeypointSize; if (dataPP) *dataPP = m_customKeypointData; return (NULL != m_customKeypointData ? true : false);}
    DGNPLATFORM_EXPORT void SetCustomKeypoint(int nBytes, Byte* dataP);

public:
    DGNPLATFORM_EXPORT bool IsHot() const;
    DGNPLATFORM_EXPORT bool IsPointOnCurve() const;
    DGNPLATFORM_EXPORT SnapHeat GetHeat() const;
    DGNPLATFORM_EXPORT DPoint3dCR GetAdjustedPoint() const;
    DGNPLATFORM_EXPORT DPoint3dCR GetSnapPoint() const;
    DGNPLATFORM_EXPORT int GetSnapDivisor() const;
    DGNPLATFORM_EXPORT double GetMinScreenDist() const;
    DGNPLATFORM_EXPORT Point2d const& GetScreenPoint() const;

    DGNPLATFORM_EXPORT SnapMode GetSnapMode() const;
    DGNPLATFORM_EXPORT SnapMode GetOriginalSnapMode() const;

    DGNPLATFORM_EXPORT bool PointWasAdjusted() const;

    DGNPLATFORM_EXPORT void SetSnapMode(SnapMode s, bool isOriginal=true);
    DGNPLATFORM_EXPORT void SetSnapDivisor(int divisor);
    DGNPLATFORM_EXPORT void SetAdjustedPoint(DPoint3dCR adjustedPt);
    DGNPLATFORM_EXPORT void SetHeat(SnapHeat isHot);
};

//=======================================================================================
// @bsiclass                                                      KeithBentley    04/01
//=======================================================================================
struct IntersectDetail : SnapDetail
{
    DEFINE_T_SUPER(SnapDetail)
private:
    HitDetail*    m_secondHit;

    virtual void _DrawInVp(DgnViewportR, DgnDrawMode drawMode, DrawPurpose drawPurpose, bool* stopFlag) const override;
    virtual HitDetailType _GetHitType() const override{return HitDetailType::Intersection;}
    DGNPLATFORM_EXPORT virtual void _SetHilited(DgnElement::Hilited) const override;
    DGNPLATFORM_EXPORT virtual bool _IsSameHit(HitDetailCP otherHit) const override;
    virtual SnapDetail* _Clone() const override;

public:
    DGNPLATFORM_EXPORT IntersectDetail(HitDetailCP firstHit, HitDetailCP secondHit, DPoint3dCR intersctionPt);
    DGNPLATFORM_EXPORT IntersectDetail(IntersectDetail const&);
    DGNPLATFORM_EXPORT ~IntersectDetail();
    HitDetail* GetSecondHit() const {return m_secondHit;}
}; 

END_BENTLEY_DGNPLATFORM_NAMESPACE
