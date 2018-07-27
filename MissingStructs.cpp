/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnSqlFuncs.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IModelBank.h"
#include "MissingStructs.h"

BEGIN_UNNAMED_NAMESPACE
static const double halfMillimeter() { return .5 * s_oneKilometer; }
static void fixRange(double &low, double &high)
{
  if (low == high)
  {
    low -= halfMillimeter();
    high += halfMillimeter();
  }
}
static bool isFixedRange(float low, float high) { return abs((high - low) - s_oneKilometer) <= 0.0001; }
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Placement3d::TryApplyTransform(TransformCR t1)
{
  if (t1.IsIdentity())
    return true;
  Transform t0 = m_angles.ToTransform(m_origin);
  return YawPitchRollAngles::TryFromTransform(m_origin, m_angles, Transform::FromProduct(t1, t0));
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Placement3d::IsMinimumRange(FPoint3dCR low, FPoint3dCR high, bool is2d)
{
  // Tile generator needs to be able to tell if an entry in the range index comes from an element with an empty
  // range (e.g., a point primitive / zero-length line) which was therefore 'fixed' by Placement3d::CalculateRange().
  // Otherwise we end up excluding those elements as 'too small' to contribute to the tile.
  return isFixedRange(low.x, high.x) && isFixedRange(low.y, high.y) && (is2d || isFixedRange(low.z, high.z));
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d Placement3d::CalculateRange() const
{
  if (!IsValid())
    return AxisAlignedBox3d();

  AxisAlignedBox3d range;
  GetTransform().Multiply(range, m_boundingBox);

  // low and high are not allowed to be equal
  fixRange(range.low.x, range.high.x);
  fixRange(range.low.y, range.high.y);
  fixRange(range.low.z, range.high.z);

  return range;
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d Placement2d::CalculateRange() const
{
  if (!IsValid())
    return AxisAlignedBox3d();

  AxisAlignedBox3d range;
  GetTransform().Multiply(range, DRange3d::From(&m_boundingBox.low, 2, 0.0));

  // low and high are not allowed to be equal
  fixRange(range.low.x, range.high.x);
  fixRange(range.low.y, range.high.y);

  range.low.z = -s_oneKilometer; // Render::Target::Get2dFrustumDepth(); // this makes range span all possible display priority values
  range.high.z = s_oneKilometer; // Render::Target::Get2dFrustumDepth();

  return range;
}
