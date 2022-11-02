/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_DGN_NAMESPACE

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
// @bsiclass
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
// @bsiclass
//=======================================================================================
enum SnapHeat
{
    SNAP_HEAT_None       = 0,
    SNAP_HEAT_NotInRange = 1,   // "of interest", but out of range
    SNAP_HEAT_InRange    = 2,
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ISnapProcessor
{
    virtual void _CorrectClosePoint(DPoint3dR point) = 0;
    virtual bool _AnnounceICurvePrimitive(ICurvePrimitiveCR curve, DPoint3dCR localPoint, HitGeomType geomType, HitParentGeomType parentGeomType) = 0;
    virtual bool _AnnounceCurveVector(CurveVectorCR curve, DPoint3dCR localPoint, HitGeomType geomType, HitParentGeomType parentGeomType) = 0;
    virtual bool _IsCanceled() const = 0;
};

//=======================================================================================
//! Interface to be used when snapping.
//=======================================================================================
namespace SnapContext
{
    struct Request  {
        BeJsConst m_value;
        Request(BeJsConst val): m_value(val) {}
        BE_JSON_NAME(id)
        BE_JSON_NAME(testPoint)
        BE_JSON_NAME(closePoint)
        BE_JSON_NAME(worldToView)
        BE_JSON_NAME(viewFlags)
        BE_JSON_NAME(snapModes)
        BE_JSON_NAME(snapAperture)
        BE_JSON_NAME(snapDivisor)
        BE_JSON_NAME(subCategoryId)
        BE_JSON_NAME(geometryClass)
        BE_JSON_NAME(intersectCandidates)
        BE_JSON_NAME(decorationGeometry)
        BE_JSON_NAME(geometryStream)
        bool IsValid() const {return m_value.isMember(json_id()) && m_value.isMember(json_closePoint()) && m_value.isMember(json_worldToView());}
        DgnElementId GetElementId() const {DgnElementId elementId; elementId.FromJson(m_value[json_id()]); return elementId;}
        DMatrix4d GetWorldToView() const {return BeJsGeomUtils::ToDMatrix4d(m_value[json_worldToView()]);}
        DPoint3d GetTestPoint() const {return BeJsGeomUtils::ToDPoint3d(m_value[json_testPoint()]);}
        DPoint3d GetClosePoint() const {return BeJsGeomUtils::ToDPoint3d(m_value[json_closePoint()]);}
        Render::ViewFlags GetViewFlags() const {Render::ViewFlags viewFlags; if (m_value.isMember(json_viewFlags())) viewFlags.FromJson(m_value[json_viewFlags()]); return viewFlags;}
        double GetSnapAperture() const {return m_value[json_snapAperture()].asDouble(12.0);}
        uint32_t GetSnapDivisor() const {return m_value[json_snapDivisor()].asUInt(2);}
        bool GetSubCategoryId(DgnElementId& subCategoryId) const {auto value = m_value[json_subCategoryId()]; if (value.isNull()) return false; subCategoryId.FromJson(value); return subCategoryId.IsValid();}
        bool GetGeometryClass(uint32_t& geomClass) const {auto value = m_value[json_geometryClass()]; if (value.isNull()) return false; geomClass = value.asUInt(); return true;}

        bset<SnapMode> GetSnapModes() const {
            bset<SnapMode> snapModes;
            auto modes = m_value[json_snapModes()];
            if (modes.isNull() || !modes.isArray()) {
              snapModes.insert(SnapMode::Nearest); // Default when a snapMode isn't supplied is nearest...
              return snapModes;
            }
            uint32_t nEntries = (uint32_t) modes.size();
            for (uint32_t i=0; i < nEntries; i++) {
                SnapMode mode = (SnapMode) modes[i].asUInt();
                switch (mode) { // Filter out exotic snap modes...
                    case SnapMode::Nearest:
                    case SnapMode::NearestKeypoint:
                    case SnapMode::MidPoint:
                    case SnapMode::Center:
                    case SnapMode::Origin:
                    case SnapMode::Bisector:
                    case SnapMode::Intersection:
                      snapModes.insert(mode);
                      break;
                }
            }
        return snapModes;
        }

        DgnElementIdSet GetIntersectCandidates(DgnElementId hitId) const {
            DgnElementIdSet elements;
            auto candidates = m_value[json_intersectCandidates()];
            if (candidates.isNull() || !candidates.isArray())
                return elements;
            uint32_t nEntries = (uint32_t) candidates.size();
            for (uint32_t i=0; i < nEntries; i++) {
                DgnElementId elemId;
                elemId.FromJson(candidates[i]);
                if (elemId != hitId)
                    elements.insert(elemId);
            }
        return elements;
        }

        std::map<DgnElementId, BeJsDocument> GetNonElementGeometry() const {
            std::map<DgnElementId, BeJsDocument> decoGeomMap;
            auto decoGeom = m_value[json_decorationGeometry()];
            if (decoGeom.isNull() || !decoGeom.isArray())
                return decoGeomMap;
            uint32_t nEntries = (uint32_t) decoGeom.size();
            for (uint32_t i=0; i < nEntries; i++) {
                DgnElementId id;
                id.FromJson(decoGeom[i][json_id()]);
                if (!id.IsValid())
                    continue;
                decoGeomMap[id].From(decoGeom[i][json_geometryStream()]);
            }
        return decoGeomMap;
        }
    };

    struct Response {
        BeJsValue m_value;
        Response(BeJsValue val) : m_value(val) {}
        BE_JSON_NAME(status)
        BE_JSON_NAME(snapMode)
        BE_JSON_NAME(heat)
        BE_JSON_NAME(geomType)
        BE_JSON_NAME(parentGeomType)
        BE_JSON_NAME(hitPoint)
        BE_JSON_NAME(snapPoint)
        BE_JSON_NAME(normal)
        BE_JSON_NAME(curve)
        BE_JSON_NAME(intersectCurve)
        BE_JSON_NAME(intersectId)
        void SetStatus(SnapStatus val) {m_value[json_status()] = (uint32_t) val;}
        void SetSnapMode(SnapMode val) {m_value[json_snapMode()] = (uint32_t) val;}
        void SetHitPoint(DPoint3dCR pt) { BeJsGeomUtils::DPoint3dToJson(m_value[json_hitPoint()], pt);}
        void SetSnapPoint(DPoint3dCR pt) { BeJsGeomUtils::DPoint3dToJson(m_value[json_snapPoint()], pt);}
        void SetHeat(SnapHeat val) {m_value[json_heat()] = (uint32_t) val;}
        void SetGeomType(HitGeomType val) {m_value[json_geomType()] = (uint32_t)  val;}
        void SetParentGeomType(HitParentGeomType val) {m_value[json_parentGeomType()] = (uint32_t) val;}
        void SetNormal(DVec3dCR val) {BeJsGeomUtils::DVec3dToJson(m_value[json_normal()], val);}
        void SetCurve(BeJsConst val) {m_value[json_curve()].From(val);}
        void SetIntersectCurve(BeJsConst val) {m_value[json_intersectCurve()].From(val);}
        void SetIntersectId(DgnElementId id) {m_value[json_intersectId()] = id.ToHexStr();}
    };

    //! Get the KeypointType for a SnapMode.
    DGNPLATFORM_EXPORT KeypointType GetSnapKeypointType(SnapMode);

    //! Compute the closest keypoint location on a line segment to the supplied fraction parameter and given keypoint divisor.
    DGNPLATFORM_EXPORT void GetSegmentKeypoint(DPoint3dR hitPoint, double& keyParam, uint32_t divisor, DSegment3dCR segment);

    //! Compute the closest keypoint location on a non-linear curve primitive (arc/bspline curve) to the supplied fraction parameter and given keypoint divisor.
    DGNPLATFORM_EXPORT bool GetParameterKeypoint(ICurvePrimitiveCR curve, DPoint3dR hitPoint, double& keyParam, uint32_t divisor);

    //! Define the current snap information as a json value given a close point in world and snap settings supplied as a json value.
    DGNPLATFORM_EXPORT void DoSnap(BeJsValue out, BeJsConst input, DgnDbR db, ICancellableR);
} // namespace SnapContext

END_BENTLEY_DGN_NAMESPACE
