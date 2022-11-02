/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <PlacementOnEarth/Placement.h>

BEGIN_UNNAMED_NAMESPACE
static const double halfMillimeter() { return .5 * PlacementOnEarth::OneMillimeter(); }
static void fixRange(double& low, double& high) {
    if (low == high) {
        low -= halfMillimeter();
        high += halfMillimeter();
    }
}
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Placement3d::TryApplyTransform(TransformCR t1) {
    if (t1.IsIdentity())
        return true;
    Transform t0 = m_angles.ToTransform(m_origin);
    return YawPitchRollAngles::TryFromTransform(m_origin, m_angles, Transform::FromProduct(t1, t0));
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Placement3d::EnsureMinimumRange(DRange3dR range) {
  // low and high are not allowed to be equal
  fixRange(range.low.x, range.high.x);
  fixRange(range.low.y, range.high.y);
  fixRange(range.low.z, range.high.z);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d Placement3d::CalculateRange() const {
    if (!IsValid())
        return AxisAlignedBox3d();

    AxisAlignedBox3d range;
    GetTransform().Multiply(range, m_boundingBox);

    EnsureMinimumRange(range);

    return range;
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Placement3d::ToJson(BeJsValue val) const {
    val.SetEmptyObject();
    BeJsGeomUtils::DPoint3dToJson(val[json_origin()], m_origin);
    BeJsGeomUtils::YawPitchRollToJson(val[json_angles()], m_angles);
    BeJsGeomUtils::DRange3dToJson(val[json_bbox()], m_boundingBox);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Placement3d::FromJson(BeJsConst val) {
    BeJsGeomUtils::DPoint3dFromJson(m_origin, val[json_origin()]);
    m_angles = BeJsGeomUtils::YawPitchRollFromJson(val[json_angles()]);
    BeJsGeomUtils::DRange3dFromJson(m_boundingBox, val[json_bbox()]);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d Placement2d::CalculateRange() const {
    if (!IsValid())
        return AxisAlignedBox3d();

    AxisAlignedBox3d range;
    GetTransform().Multiply(range, DRange3d::From(&m_boundingBox.low, 2, 0.0));

    // low and high are not allowed to be equal
    fixRange(range.low.x, range.high.x);
    fixRange(range.low.y, range.high.y);

    range.low.z = -PlacementOnEarth::OneMeter(); // Render::Target::Get2dFrustumDepth(); // this makes range span all possible display priority values
    range.high.z = PlacementOnEarth::OneMeter(); // Render::Target::Get2dFrustumDepth();

    return range;
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Placement2d::ToJson(BeJsValue val) const {
    val.SetEmptyObject();
    BeJsGeomUtils::DPoint2dToJson(val[json_origin()], m_origin);
    BeJsGeomUtils::AngleInDegreesToJson(val[json_angle()], m_angle);
    BeJsGeomUtils::DRange2dToJson(val[json_bbox()], m_boundingBox);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Placement2d::FromJson(BeJsConst val) {
    BeJsGeomUtils::DPoint2dFromJson(m_origin, val[json_origin()]);
    m_angle = BeJsGeomUtils::AngleInDegreesFromJson(val[json_angle()]);
    BeJsGeomUtils::DRange2dFromJson(m_boundingBox, val[json_bbox()]);
}
