/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/HitDetail.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/bvector.h>
#include "ISprite.h"
#include "ElementGeometry.h"
#include "Render.h"

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
enum class SubSelectionMode
    {
    None        = 0, //! Select entire element - No sub-selection
    Part        = 1, //! Select single DgnGeometryPart
    Primitive   = 2, //! Select single GeometricPrimitive
    Segment     = 3, //! Select single ICurvePrimitive/line string segment of open paths, and planar regions.
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
    TextBox     = 500,
    Region      = 550,
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
//! What was being tested to generate this hit. This is not the element or 
//! GeometricPrimitive that generated the Hit, it's an indication of whether it's an
//! edge or interior hit.
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
//! Indicates whether the GeometricPrimitive that generated the hit was a wire,
//! surface, or solid.
//=======================================================================================
enum class HitParentGeomType
{
    None           = 0,
    Wire           = 1,
    Sheet          = 2,
    Solid          = 3,
    Mesh           = 4,
    Text           = 5,
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
    ICurvePrimitivePtr m_primitive;     // curve primitve for hit (world coordinates).
    DPoint3d m_closePoint;              // the closest point on geometry (world coordinates).
    DVec3d m_normal;                    // surface hit normal (world coordinates).
    HitParentGeomType m_parentType;     // type of parent geometry.
    HitGeomType m_geomType;             // type of hit geometry (edge or interior).
    HitDetailSource m_detailSource;     // mask of HitDetailSource values.
    HitPriority m_hitPriority;          // Relative priority of hit.
    bool m_nonSnappable;                // non-snappable detail, ex. pattern or line style.
    double m_viewDist;                  // xy distance to hit (view coordinates).
    double m_viewZ;                     // z distance to hit (view coordinates).
    GeometryStreamEntryId m_geomId;     // id of geometric primitive that generated this hit.

public:
    DGNPLATFORM_EXPORT void Init();

    DPoint3dCR GetClosestPoint() const {return m_closePoint;}
    DVec3dCR GetSurfaceNormal() const {return m_normal;}
    HitParentGeomType GetParentGeomType() const {return m_parentType;}
    HitGeomType GetGeomType() const {return m_geomType;}
    HitDetailSource GetDetailSource() const {return m_detailSource;}
    HitPriority GetLocatePriority() const {return m_hitPriority;}
    bool IsSnappable() const {return !m_nonSnappable;}
    double GetScreenDist() const {return m_viewDist;}
    double GetZValue() const {return m_viewZ;}

    void SetClosestPoint(DPoint3dCR pt) {m_closePoint = pt;}
    void SetSurfaceNormal(DVec3dCR value) {m_normal = value;}
    void SetParentGeomType(HitParentGeomType value) {m_parentType = value;}
    void SetGeomType(HitGeomType value) {m_geomType = value; m_primitive = nullptr;} // NOTE: Use SetCurvePrimitive for HitGeomType::Segment/Arc/Curve.
    void SetDetailSource(HitDetailSource value) {m_detailSource = value;}
    void SetLocatePriority(HitPriority value) {m_hitPriority = value;}
    void SetNonSnappable(bool yesNo) {m_nonSnappable = yesNo;}
    void SetScreenDist(double value) {m_viewDist = value;}
    void SetZValue(double value) {m_viewZ = value;}

    //! @private
    GeometryStreamEntryId GetGeometryStreamEntryId(bool wantPartIndex=false) const {if (wantPartIndex) return m_geomId; GeometryStreamEntryId tmpId = m_geomId; tmpId.SetPartIndex(0); return tmpId;}
    //! @private
    void SetGeometryStreamEntryId(GeometryStreamEntryId geomId) {m_geomId = geomId;}

    //! <ul>
    //! <li>Return the current geometry as an ICurvePrimitive.
    //! <li>If singleSegment is requested and the current geometry is a segment select within a linestring, only return that segment.
    //! </ul>
    DGNPLATFORM_EXPORT bool GetCurvePrimitive (ICurvePrimitivePtr &curve, bool singleSegment)const;
    DGNPLATFORM_EXPORT bool GetArc(DEllipse3dR) const;
    DGNPLATFORM_EXPORT bool GetSegment(DSegment3dR) const;
    DGNPLATFORM_EXPORT size_t GetSegmentNumber() const;
    DGNPLATFORM_EXPORT double GetSegmentParam() const;
    DGNPLATFORM_EXPORT double GetCloseParam() const;
    DGNPLATFORM_EXPORT size_t GetCloseVertex() const;
    DGNPLATFORM_EXPORT size_t GetPointCount() const;
    DGNPLATFORM_EXPORT bool IsValidSurfaceHit() const; // Test for HitGeomType::Surface with valid normal (i.e. not a QVElem wireframe edge hit)...
    DGNPLATFORM_EXPORT bool IsValidEdgeHit() const; // Check hit types, HitGeomType::Segment/HitGeomType::Curve/HitGeomType::Arc (is GetCloseParam, etc. meaningful?)...
    DGNPLATFORM_EXPORT bool GetCloseDetail(CurveLocationDetailR location) const;

    DGNPLATFORM_EXPORT ICurvePrimitiveCP GetCurvePrimitive() const;
    DGNPLATFORM_EXPORT CurvePrimitiveIdCP GetCurvePrimitiveId() const;
    DGNPLATFORM_EXPORT HitGeomType GetCurvePrimitiveType() const;
    DGNPLATFORM_EXPORT HitGeomType GetEffectiveHitGeomType() const; // Return GetGeomType or GetCurvePrimitiveType for HitGeomType::Surface. 

    //! Sets ICurvePrimitive hit geometry and appropriate HitGeomType for the supplied primitive.
    //! @note Optional geomType can be explicity specified to override the default HitGeomType.
    //!       For example, an arc primitive with HitGeomType::Point denotes a hit on the arc center.
    DGNPLATFORM_EXPORT void SetCurvePrimitive(ICurvePrimitiveCP curve, TransformCP localToWorld = nullptr, HitGeomType geomType = HitGeomType::None);

    //! @private
    DGNPLATFORM_EXPORT void ClipCurvePrimitive(ClipVectorCP);
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

END_BENTLEY_DGN_NAMESPACE
