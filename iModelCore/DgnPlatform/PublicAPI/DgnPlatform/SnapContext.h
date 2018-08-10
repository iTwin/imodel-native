/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/SnapContext.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
namespace SnapContext
{
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

    //! Get the KeypointType for a SnapMode.
    DGNPLATFORM_EXPORT KeypointType GetSnapKeypointType(SnapMode);

    //! Get the default sprite used for the supplied SnapMode.
    //! @param[in]    mode                  Snap mode to get sprite for.
    //! @return ISpriteP default sprite.
    DGNPLATFORM_EXPORT Render::ISpriteP GetSnapSprite(SnapMode mode);

    //! Compute the closest keypoint location on a line segment to the supplied fraction parameter and given keypoint divisor.
    DGNPLATFORM_EXPORT void GetSegmentKeypoint(DPoint3dR hitPoint, double& keyParam, int divisor, DSegment3dCR segment);

    //! Compute the closest keypoint location on a non-linear curve primitive (arc/bspline curve) to the supplied fraction parameter and given keypoint divisor.
    DGNPLATFORM_EXPORT bool GetParameterKeypoint(ICurvePrimitiveCR curve, DPoint3dR hitPoint, double& keyParam, int divisor);

    //! Define the current snap information as a json value given a close point in world and snap settings supplied as a json value.
    DGNPLATFORM_EXPORT Response DoSnap(Request const& input, DgnDbR db, struct CheckStop&);

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
    DGNPLATFORM_EXPORT void SetSnapInfo(SnapDetailR snap, SnapMode mode, Render::ISpriteP sprite, DPoint3dCR snapPoint, bool forceHot, double aperture, bool isAdjusted, int nBytes = 0, Byte* customKeypointData = nullptr);
} // namespace SnapContext

END_BENTLEY_DGN_NAMESPACE
