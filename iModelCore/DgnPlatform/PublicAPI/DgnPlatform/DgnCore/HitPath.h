/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/HitPath.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/bvector.h>
#include "ISprite.h"
#include "DisplayPath.h"
#include "IViewDraw.h"
#include <DgnPlatform/DgnHandlers/DimensionStyleProps.r.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//!  Lower numbers are "better" (more important) Hits than ones with higher numbers.
//=======================================================================================
enum class HitPriority
{
    Highest     = 0,
    Vertex      = 300,
    Origin      = 400,
    CellOrigin  = 400,
    Edge        = 400,
    Compound    = 500,
    TestBox     = 550,
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
//!  Hit detail type can be used to tell what display attribute of an
//!      element generated the hit geometry
//=======================================================================================
enum HitDetailType
{
    HIT_DETAIL_None         = 0,
    HIT_DETAIL_LineStyle    = 1,
    HIT_DETAIL_Pattern      = 1 << 1,
    HIT_DETAIL_Thickness    = 1 << 2,
    HIT_DETAIL_PointCloud   = 1 << 3,
    HIT_DETAIL_Sprite       = 1 << 4,
};

//struct IViewHandlerHitInfo; removed in graphite

//=======================================================================================
// @bsiclass                                                      KeithBentley    04/01
//=======================================================================================
struct  SelectionPath : DisplayPath
{
//__PUBLISH_SECTION_END__
private:

    DgnViewportP  m_viewport;

public:

    explicit SelectionPath (DgnViewportP vp) {SetViewport (vp);}
    DGNPLATFORM_EXPORT explicit SelectionPath (SelectionPath const* from);

    void SetViewport (DgnViewportP view) {m_viewport = view;}
    DgnViewportP GetViewport () const {return m_viewport;}
    DGNPLATFORM_EXPORT int GetViewNumber () const;

    virtual DisplayPathType GetPathType () const override {return DisplayPathType::Selection;}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

}; // SelectionPath

//=======================================================================================
//!  Hit detail element arg defines
//=======================================================================================
enum    BCurveElemArg
{
    BCURVE_ELEMARG_None             = 0,
    BCURVE_ELEMARG_ControlPolygon   = 1,
    BCURVE_ELEMARG_FitCurvePolygon  = 2,
    BCURVE_ELEMARG_StartTangent     = 3,
    BCURVE_ELEMARG_EndTangent       = 4,
};

enum    BSurfElemArg
{
    BSURF_ELEMARG_None              = 0,
    BSURF_ELEMARG_ControlPolygon    = 2,
};

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  05/12
+===============+===============+===============+===============+===============+======*/
struct MeshSegmentInfo : RefCounted <ICurvePrimitiveInfo>
    {
    uint32_t m_closeVertex;
    uint32_t m_segmentVertex;

    MeshSegmentInfo (int32_t closeVertex, uint32_t segmentVertex) {m_closeVertex = closeVertex, m_segmentVertex = segmentVertex;}

    DGNPLATFORM_EXPORT static ICurvePrimitiveInfoPtr Create (uint32_t closeVertex, uint32_t segmentVertex);

    }; // MeshSegmentInfo

//=======================================================================================
// @bsiclass                                                      Keith.Bentley   10/04
//=======================================================================================
struct  GeomDetail
{
private:

    ICurvePrimitivePtr      m_primitive;                // curve primitve for hit in local coords.
    DMatrix4d               m_localToWorld;             // local to world transform for hit.
    DPoint3d                m_localPoint;               // the closest point in geometry's local coordinates.
    HitGeomType             m_geomType;                 // category hit geometry falls into.
    uint32_t            m_detailType;       // mask of HitDetailType values.
    HitPriority             m_hitPriority;              // Relative priority of hit.
    uint32_t            m_patternIndex;     // pattern index (needed for mlines).
    int                     m_elemArg;                  // meaning varies according to element type
    bool                    m_nonSnappable;             // non-snappable detail, ex. pattern or line style.
    double                  m_viewDist;                 // xy distance to hit in view coordinates.
    double                  m_viewZ;                    // z distance to hit in view coordinates.
    DVec3d                  m_localNormal;              // surface hit normal in local coordinates.

public:

    DGNPLATFORM_EXPORT void Init ();

    DMatrix4dCR             GetLocalToWorld () const        {return m_localToWorld;}
    DPoint3dCR              GetClosestPointLocal () const   {return m_localPoint;}
    HitGeomType             GetGeomType () const            {return m_geomType;}
    int                     GetDetailType () const          {return m_detailType;}
    HitPriority             GetLocatePriority () const      {return m_hitPriority;}
    int                     GetPatternIndex () const        {return m_patternIndex;}
    int                     GetElemArg () const             {return m_elemArg;}
    bool                    IsSnappable () const            {return !m_nonSnappable;}
    double                  GetScreenDist () const          {return m_viewDist;}
    double                  GetZValue () const              {return m_viewZ;}
    DVec3dCR                GetSurfaceNormal () const       {return m_localNormal;}

    void                    SetLocalToWorld (DMatrix4dCR matrix)   {m_localToWorld = matrix;}
    void                    SetClosestPointLocal (DPoint3dCR pt)   {m_localPoint = pt;}
    void                    SetGeomType (HitGeomType value)        {m_geomType = value; m_primitive = NULL;} // NOTE: Use SetCurvePrimitive for HitGeomType::Segment/Arc/Curve.
    void                    SetDetailType (int value)              {m_detailType = value;}
    void                    SetLocatePriority (HitPriority value)  {m_hitPriority = value;}
    void                    SetPatternIndex (int index)            {m_patternIndex = index;}
    void                    SetElemArg (int value)                 {m_elemArg = value;}
    void                    SetNonSnappable (bool yesNo)           {m_nonSnappable = yesNo;}
    void                    SetScreenDist (double value)           {m_viewDist = value;}
    void                    SetZValue (double value)               {m_viewZ = value;}
    void                    SetSurfaceNormal (DVec3dCR value)      {m_localNormal = value;}

    DGNPLATFORM_EXPORT void GetClosestPoint (DPoint3dR pt) const;
    DGNPLATFORM_EXPORT void SetClosestPoint (DPoint3dCR pt);

    DGNPLATFORM_EXPORT bool     FillGPA (GPArrayR, bool worldCoords = true, bool singleSegment = true) const;
    DGNPLATFORM_EXPORT bool     GetArc (DEllipse3dR) const;
    DGNPLATFORM_EXPORT bool     GetSegment (DSegment3dR) const;
    DGNPLATFORM_EXPORT size_t   GetSegmentNumber () const;
    DGNPLATFORM_EXPORT double   GetSegmentParam () const;
    DGNPLATFORM_EXPORT double   GetCloseParam () const;
    DGNPLATFORM_EXPORT size_t   GetCloseVertex () const;
    DGNPLATFORM_EXPORT size_t   GetPointCount () const;
    DGNPLATFORM_EXPORT bool     IsValidSurfaceHit () const; // Test for HitGeomType::Surface with valid normal (i.e. not a QVElem wireframe edge hit)...
    DGNPLATFORM_EXPORT bool     IsValidEdgeHit () const; // Check path types, HitGeomType::Segment/HitGeomType::Curve/HitGeomType::Arc (is GetCloseParam, etc. meaningful?)...

    DGNPLATFORM_EXPORT ICurvePrimitiveCP   GetCurvePrimitive () const;
    DGNPLATFORM_EXPORT CurvePrimitiveIdCP  GetCurvePrimitiveId () const;
    DGNPLATFORM_EXPORT HitGeomType         GetCurvePrimitiveType () const;
    DGNPLATFORM_EXPORT HitGeomType         GetEffectiveHitGeomType () const; // Return GetGeomType or GetCurvePrimitiveType for HitGeomType::Surface. 

    //! Sets ICurvePrimitive hit geometry and appropriate HitGeomType for the supplied primitive.
    //! @note Optional geomType can be explicity specified to override the default HitGeomType.
    //!       For example, an arc primitive with HitGeomType::Point denotes a hit on the arc center.
    DGNPLATFORM_EXPORT void SetCurvePrimitive (ICurvePrimitiveCP curve, HitGeomType geomType = HitGeomType::None);
};

//=======================================================================================
//! @ingroup HitPaths
// @bsiclass                                                      KeithBentley    04/01
//=======================================================================================
struct HitPath : SelectionPath
{
    DEFINE_T_SUPER(SelectionPath)

//__PUBLISH_SECTION_END__
protected:

    HitSource                   m_locateSource;         // Operation that generated the hit.
    DPoint3d                    m_testPoint;            // the point that was used to search (world coordinates).
    ViewFlags                   m_viewFlags;            // view flags in effect when hit was generated.
    GeomDetail                  m_geomDetail;           // element specific hit details.
    IElemTopologyCP             m_elemTopo;             // details about the topology of the element.
    bool                        m_componentMode;        // component hilite/flash mode.

    void ClearElemTopology ();

    virtual void _GetInfoString (Utf8StringR pathDescr, Utf8CP delimiter) const override;

public: // WIP_PUBLIC_VIRTUAL - these methods should be protected and start with _
    bool GetComponentMode () const {return m_componentMode; }
    void SetComponentMode (bool componentMode) {m_componentMode = componentMode; }

    virtual void            GetHitPoint (DPoint3dR pt) const {return m_geomDetail.GetClosestPoint (pt);}
    virtual void            SetHitPoint (DPoint3dCR pt) {m_geomDetail.SetClosestPoint (pt);}
    virtual void            SetTestPoint (DPoint3dCR pt) {m_testPoint = pt;}
    virtual bool            IsSamePath (DisplayPathCP otherPath, bool fullPath) const override;
    virtual DisplayPathType GetPathType () const override {return DisplayPathType::Hit;}

public:
    DGNPLATFORM_EXPORT HitPath (DgnViewportP, DisplayPathCR, DPoint3dCR testPoint, HitSource, ViewFlagsCR, GeomDetailCR, IElemTopologyCP = NULL);
    DGNPLATFORM_EXPORT StatusInt GetHitLocalToContextLocal (TransformR, ViewContextR) const;
    DGNPLATFORM_EXPORT StatusInt GetContextLocalToHitLocal (TransformR, ViewContextR) const;
    DGNPLATFORM_EXPORT explicit HitPath (HitPathCR from);
    DGNPLATFORM_EXPORT virtual ~HitPath ();

    void SetLocateSource (HitSource source) {m_locateSource = source;}

    DGNPLATFORM_EXPORT StatusInt OnCreateAssociationToSnap (DgnModelP);
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    DGNPLATFORM_EXPORT HitSource       GetLocateSource () const;
    DGNPLATFORM_EXPORT DPoint3dCR      GetTestPoint () const;
    DGNPLATFORM_EXPORT GeomDetailCR    GetGeomDetail () const;
    DGNPLATFORM_EXPORT GeomDetailR     GetGeomDetailW ();
    DGNPLATFORM_EXPORT ViewFlagsCR     GetViewFlags () const;
    DGNPLATFORM_EXPORT IElemTopologyCP GetElemTopology () const;
    DGNPLATFORM_EXPORT void            SetElemTopology (IElemTopologyP topo);

    //! Get the parameters from a HitPath, if it is a hit of a linear element
    //! @param[out]         hitSeg          The coordinates of the segment on the element that caused the hit.
    //! @param[out]         vertex          The vertex number of the closest vertex to the test point.
    //! @param[out]         segmentNumber   The segment number of hitSeg.
    //! @return       SUCCESS or ERROR
    DGNPLATFORM_EXPORT StatusInt GetLinearParameters (DSegment3dP hitSeg, int* vertex, int* segmentNumber) const;

    //! Get the parameters from a HitPath, if it is a hit of a Multiline element.
    //! @param[out]         hitSeg          The coordinates of the segment on the element that caused the hit.
    //! @param[out]         vertex          The vertex number of the closest vertex to the test point
    //! @param[out]         segmentNumber   The segment number of hitSeg.
    //! @param[out]         lineNumber      The work line within the multiline.
    //! @param[out]         capNumber       The cap number on the multiline.
    //! @param[out]         patIndex        Pattern index.
    DGNPLATFORM_EXPORT StatusInt GetMultilineParameters (DSegment3dP hitSeg, int* vertex, int* segmentNumber, int* lineNumber, int* capNumber,int* patIndex) const;

    //! Get the parameters from a Hitpath, if it is a hit of a Bspline
    //! @param[out]         u               U
    //! @param[out]         v               V
    //! @return       SUCCESS or ERROR
    DGNPLATFORM_EXPORT StatusInt GetBSplineParameters (double* u, double* v) const;

    //! Get the parameters from a Hitpath, if it is a hit of a Dimension.
    //! @param[out]         partName        Complete part identifier for part of dimension located, use ADIM_GETTYPE, ADIM_GETSUB and ADIM_GETSEG to decompose.
    //! @param[out]         pointNo         Closest point on dimension element.
    //! @param[out]         segment         Selected dimension segment.
    //! @param[out]         partType        Type of selected part of dim, one of ADTYPE_xxx defined in mdldim.h.
    //! @param[out]         partSubType     Sub Type of selected part of dim, one of ADSUB_xxx defined in mdldim.h.
    //! @return       SUCCESS or ERROR
    DGNPLATFORM_EXPORT StatusInt GetDimensionParameters (uint32_t* partName,int* pointNo, uint32_t* segment, DimensionPartType* partType, DimensionPartSubType* partSubType) const;

}; // HitPath

typedef bvector<RefCountedPtr<SelectionPath>> SelectionPathArray;

//=======================================================================================
//! The result of a "locate" is a sorted list of objects that
//! satisfied the search  criteria (a HitList). Earlier hits in the list
//! are somehow "better" than those later on.
//! @see          IHitPath
// @bsiclass                                                      KeithBentley    04/01
//=======================================================================================
struct  HitList : private SelectionPathArray
{
    // NOTE: use private inheritance -- we don't want anybody calling the base class "empty" method!
    int             m_currHit;
public:
    DGNPLATFORM_EXPORT HitList    ();
    DGNPLATFORM_EXPORT virtual ~HitList   ();

    DGNPLATFORM_EXPORT void RemoveHit (int hitNum);
    DGNPLATFORM_EXPORT void RemoveCurrentHit ();
    DGNPLATFORM_EXPORT SelectionPath* GetHit (int hitNum) const;
    DGNPLATFORM_EXPORT int Compare (HitPathCP oHit1, HitPathCP oHit2, bool comparePriority, bool compareElemClass, bool compareZ) const;
    DGNPLATFORM_EXPORT void SetCurrentHit (HitPathCP);
    DGNPLATFORM_EXPORT int AddHit (HitPathP oDispPath, bool allowDuplicates, bool comparePriority, bool compareElemClass);
    DGNPLATFORM_EXPORT void Empty ();
    DGNPLATFORM_EXPORT void ResetCurrentHit () {m_currHit = -1;}
    DGNPLATFORM_EXPORT SelectionPath* GetCurrentHit () const;
    DGNPLATFORM_EXPORT SelectionPath* GetNextHit ();
    DGNPLATFORM_EXPORT bool RemoveHitsMatchingPath (DisplayPathCP path);
    DGNPLATFORM_EXPORT bool RemoveHitsContaining (DgnElementP element);
    DGNPLATFORM_EXPORT bool RemoveHitsFrom (DgnModelP modelRef);
    DGNPLATFORM_EXPORT virtual void Dump (WCharCP label) const;

    //  Because we use private inheritance, we must re-export every SelectionPathArray method that we want to expose.
    DGNPLATFORM_EXPORT int GetCount () const;
    DGNPLATFORM_EXPORT SelectionPath* Get (int i);
    DGNPLATFORM_EXPORT void Set (int i, SelectionPath*);
    DGNPLATFORM_EXPORT void Insert (int i, SelectionPath*);
    DGNPLATFORM_EXPORT void DropNulls ();
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
enum            SnapHeat
{
    SNAP_HEAT_None       = 0,
    SNAP_HEAT_NotInRange = 1,   // "of interest", but out of range
    SNAP_HEAT_InRange    = 2,
};

//=======================================================================================
// @bsiclass                                                      KeithBentley    04/01
//=======================================================================================
struct  SnapPath : HitPath
{
    DEFINE_T_SUPER(HitPath)

//__PUBLISH_SECTION_END__

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

public: // WIP_PUBLIC_VIRTUAL - these methods should be protected and start with _
    DGNPLATFORM_EXPORT virtual void GetHitPoint (DPoint3dR pt) const override;
    DGNPLATFORM_EXPORT virtual void SetHitPoint (DPoint3dCR snapPt) override;
    virtual DisplayPathType GetPathType () const override {return DisplayPathType::Snap;}
    virtual SnapPath* Clone () const;

public:
    DGNPLATFORM_EXPORT explicit SnapPath (HitPathCP from);
    DGNPLATFORM_EXPORT explicit SnapPath (SnapPath const&);
    DGNPLATFORM_EXPORT ~SnapPath ();

    void SetScreenPoint (Point2d const& pt) {m_screenPt = pt;} //!< Always set by DgnPlatform snap logic
    
    bool GetAllowAssociations () const {return m_allowAssociations;}
    void SetAllowAssociations (bool allowAssociations) {m_allowAssociations = allowAssociations;}

    ISpriteP GetSprite () const {return m_sprite;}
    void SetSprite (ISpriteP sprite) {if (m_sprite) m_sprite->Release (); m_sprite = sprite; if (m_sprite) m_sprite->AddRef ();}

    DGNPLATFORM_EXPORT bool GetCustomKeypoint (int* nBytesP, Byte** dataPP) const {if (nBytesP) *nBytesP = m_customKeypointSize; if (dataPP) *dataPP = m_customKeypointData; return (NULL != m_customKeypointData ? true : false);}
    DGNPLATFORM_EXPORT void SetCustomKeypoint (int nBytes, Byte* dataP);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    DGNPLATFORM_EXPORT bool IsHot () const;
    DGNPLATFORM_EXPORT bool IsPointOnCurve () const;
    DGNPLATFORM_EXPORT SnapHeat GetHeat () const;
    DGNPLATFORM_EXPORT DPoint3dCR GetAdjustedPoint () const;
    DGNPLATFORM_EXPORT DPoint3dCR GetSnapPoint () const;
    DGNPLATFORM_EXPORT int GetSnapDivisor () const;
    DGNPLATFORM_EXPORT double GetMinScreenDist () const;
    DGNPLATFORM_EXPORT Point2d const& GetScreenPoint () const;

    DGNPLATFORM_EXPORT SnapMode GetSnapMode () const;
    DGNPLATFORM_EXPORT SnapMode GetOriginalSnapMode () const;

    DGNPLATFORM_EXPORT bool PointWasAdjusted () const;

    DGNPLATFORM_EXPORT void SetSnapMode (SnapMode s, bool isOriginal=true);
    DGNPLATFORM_EXPORT void SetSnapDivisor (int divisor);
    DGNPLATFORM_EXPORT void SetAdjustedPoint (DPoint3dCR adjustedPt);
    DGNPLATFORM_EXPORT void SetHeat (SnapHeat isHot);

};

//=======================================================================================
// @bsiclass                                                      KeithBentley    04/01
//=======================================================================================
struct  IntersectPath : SnapPath
{
    DEFINE_T_SUPER(SnapPath)
//__PUBLISH_SECTION_END__
private:
    HitPath*    m_secondPath;
    virtual void   _DrawInVp            (DgnViewportP, DgnDrawMode drawMode, DrawPurpose drawPurpose, bool* stopFlag) const override;

public: // WIP_PUBLIC_VIRTUAL - these methods should be protected and start with _
    virtual DisplayPathType GetPathType () const override{return DisplayPathType::Intersection;}
    DGNPLATFORM_EXPORT virtual void SetHilited (DgnElement::Hilited) const override;
    DGNPLATFORM_EXPORT virtual bool IsSamePath (DisplayPathCP otherPath, bool fullPath) const override;
    virtual SnapPath* Clone () const override;

public:
    DGNPLATFORM_EXPORT IntersectPath (HitPathCP firstPath, HitPathCP secondPath, DPoint3dCR intersctionPt);
    DGNPLATFORM_EXPORT IntersectPath (IntersectPath const&);
    DGNPLATFORM_EXPORT ~IntersectPath ();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    DGNPLATFORM_EXPORT HitPath* GetSecondPath () const;
};

//__PUBLISH_SECTION_END__
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
inline HitPathCP toHitPath (DisplayPathCP dispPath)
    {
    return (dynamic_cast <HitPathCP> (dispPath));
    }

//__PUBLISH_SECTION_START__

END_BENTLEY_DGNPLATFORM_NAMESPACE
