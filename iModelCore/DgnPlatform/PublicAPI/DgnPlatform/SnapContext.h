/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "HitDetail.h"
#include "NullContext.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Interface to be used when snapping.
//=======================================================================================
struct SnapContext : NullContext
{
protected:
    SnapDetailP m_snapPath; // result of the snap
    double      m_snapAperture;
    SnapMode    m_snapMode;
    int         m_snapDivisor;

public:
    virtual ~SnapContext() {}

    struct Request : Json::Value {
        BE_JSON_NAME(id)
        BE_JSON_NAME(closePoint)
        BE_JSON_NAME(worldToView)
        BE_JSON_NAME(viewFlags)
        BE_JSON_NAME(snapMode)
        BE_JSON_NAME(snapAperture)
        BE_JSON_NAME(snapDivisor)
        BE_JSON_NAME(offSubCategories)
        bool IsValid() const {return isMember(json_id()) && isMember(json_closePoint()) && isMember(json_worldToView());}
        DgnElementId GetElementId() const {DgnElementId elementId; elementId.FromJson((*this)[json_id()]); return elementId;}
        DMatrix4d GetWorldToView() const {return JsonUtils::ToDMatrix4d((*this)[json_worldToView()]);}
        DPoint3d GetClosePoint() const {return JsonUtils::ToDPoint3d((*this)[json_closePoint()]);}
        Render::ViewFlags GetViewFlags() const {Render::ViewFlags viewFlags; if (isMember(json_viewFlags())) viewFlags.FromJson((*this)[json_viewFlags()]); return viewFlags;}
        SnapMode GetSnapMode() const {return (SnapMode) (*this)[json_snapMode()].asUInt((int) SnapMode::NearestKeypoint);}
        double GetSnapAperture() const {return (*this)[json_snapAperture()].asDouble(12.0);}
        int GetSnapDivisor() const {return (*this)[json_snapDivisor()].asUInt(2);}
        JsonValueCR GetOffSubCategories() const {return (*this)[json_offSubCategories()];}
    };

    struct Response : Json::Value {
        BE_JSON_NAME(status)
        BE_JSON_NAME(heat)
        BE_JSON_NAME(geomType)
        BE_JSON_NAME(parentGeomType)
        BE_JSON_NAME(subCategory)
        BE_JSON_NAME(weight)
        BE_JSON_NAME(snapPoint)
        BE_JSON_NAME(normal)
        BE_JSON_NAME(curve)
        BE_JSON_NAME(localToWorld)
        void SetStatus(SnapStatus val) {(*this)[json_status()] = (uint32_t) val;}
        void SetSnapPoint(DPoint3dCR pt) {(*this)[json_snapPoint()] = JsonUtils::DPoint3dToJson(pt);}
        void SetHeat(SnapHeat val) {(*this)[json_heat()] = (uint32_t) val;}
        void SetGeomType(HitGeomType val) {(*this)[json_geomType()] = (uint32_t)  val;}
        void SetParentGeomType(HitParentGeomType val) {(*this)[json_parentGeomType()] = (uint32_t) val;}
        void SetSubCategory(Utf8StringCR val) {(*this)[json_subCategory()] = val;}
        void SetWeight(uint32_t val) {(*this)[json_weight()] = val;}
        void SetNormal(DVec3dCR val) {(*this)[json_normal()] = JsonUtils::DVec3dToJson(val);}
        void SetLocalToWorld(TransformCR val) {(*this)[json_localToWorld()] = JsonUtils::FromTransform(val);}
        void SetCurve(JsonValueCR val) {(*this)[json_curve()] = val;}
    };

public:
    DGNVIEW_EXPORT SnapStatus IntersectDetails(SnapDetailP* snappedPath, HitDetailCP first, HitDetailCP second, DPoint3dCP testPoint, bool allowSelfIntersections);
    DGNVIEW_EXPORT SnapStatus SnapToPath(SnapDetailP* snappedPath, HitDetailCP thisPath, SnapMode snapMode, int snapDivisor, double hotAperture);

    //! Get the KeypointType for a SnapMode.
    DGNPLATFORM_EXPORT static KeypointType GetSnapKeypointType(SnapMode);

    //! Get the snap path created by calling element's draw method, and
    //! representing "nearest" snap.
    //! @return SnapDetailP won't return NULL.
    SnapDetailP GetSnapDetail() {return m_snapPath;}

    //! Get the snap mode that the handler is being asked to adjust the snap path for.
    //! @return SnapMode to use for this snap.
    SnapMode GetSnapMode() {return m_snapMode;}

    //! Get the current snap divisor setting, this is used for SnapMode::NearestKeypoint.
    //! @return current snap divisor.
    int GetSnapDivisor() {return m_snapDivisor;}

    //! Get hot distance to use to set snap heat.
    double GetSnapAperture() {return m_snapAperture;}

    //! Get the default sprite used for the supplied SnapMode.
    //! @param[in]    mode                  Snap mode to get sprite for.
    //! @return ISpriteP default sprite.
    DGNPLATFORM_EXPORT static Render::ISpriteP GetSnapSprite(SnapMode mode);

    //! Compute the closest keypoint location on a line segment to the supplied fraction parameter and given keypoint divisor.
    DGNPLATFORM_EXPORT static void GetSegmentKeypoint(DPoint3dR hitPoint, double& keyParam, int divisor, DSegment3dCR segment);

    //! Compute the closest keypoint location on a non-linear curve primitive (arc/bspline curve) to the supplied fraction parameter and given keypoint divisor.
    DGNPLATFORM_EXPORT static bool GetParameterKeypoint(ICurvePrimitiveCR curve, DPoint3dR hitPoint, double& keyParam, int divisor);

    //! Define the current snap information as a json value given a close point in world and snap settings supplied as a json value.
    DGNPLATFORM_EXPORT static Response DoSnap(Request const& input, DgnDbR db, struct CheckStop&);

    //! Define the current snap information for text using default processing.
    DGNPLATFORM_EXPORT SnapStatus DoTextSnap();

    //! Define the current snap information for this hit using default processing.
    DGNPLATFORM_EXPORT SnapStatus DoDefaultDisplayableSnap();

    //! Define the current snap information for the path curve primitive using default processing.
    //! @param[in] mode Snap mode to use.
    DGNPLATFORM_EXPORT SnapStatus DoSnapUsingCurve(SnapMode mode);

    //! Specify a location for this snap that is different from the point that actually generated the snap.
    DGNPLATFORM_EXPORT void SetAdjustedSnapPoint(DPoint3dCR adjustedPt);

    //! Define the current snap information for this hit.
    //! @param[in] snap SnapDetail to update.
    //! @param[in] mode Snap mode used for this snap.
    //! @param[in] sprite Sprite to use to decorate snap.
    //! @param[in] snapPoint Location for snap in world coordinates.
    //! @param[in] forceHot true to make snap active even if cursor is not within locate tolerance of snap location.
    //! @param[in] aperture Hot distance to use when forceHot is false.
    //! @param[in] isAdjusted true if snap is not suitable for creating assoc points (pass false if customKeypointData is supplied or snap not overriden).
    //! @param[in] nBytes Size in bytes of customKeypointData, or 0 if none.
    //! @param[in] customKeypointData Pointer to customKeypointData to save for this snap or NULL.
    DGNPLATFORM_EXPORT static void SetSnapInfo(SnapDetailR snap, SnapMode mode, Render::ISpriteP sprite, DPoint3dCR snapPoint, bool forceHot, double aperture, bool isAdjusted, int nBytes = 0, Byte* customKeypointData = nullptr);
};

END_BENTLEY_DGN_NAMESPACE
