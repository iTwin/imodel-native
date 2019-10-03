/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Geom/GeomApi.h>
#include <GeomJsonWireFormat/JsonUtils.h>

BENTLEY_NAMESPACE_TYPEDEFS(AxisAlignedBox2d)
BENTLEY_NAMESPACE_TYPEDEFS(AxisAlignedBox3d)
BENTLEY_NAMESPACE_TYPEDEFS(BoundingBox2d)
BENTLEY_NAMESPACE_TYPEDEFS(BoundingBox3d)
BENTLEY_NAMESPACE_TYPEDEFS(ElementAlignedBox2d)
BENTLEY_NAMESPACE_TYPEDEFS(ElementAlignedBox3d)
BENTLEY_NAMESPACE_TYPEDEFS(Placement2d)
BENTLEY_NAMESPACE_TYPEDEFS(Placement3d)

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct PlacementOnEarth
    {
    static double constexpr OneMeter() {return 1.0;}
    static double constexpr OneKilometer() {return 1000.0 * OneMeter();}
    static double constexpr OneMillimeter() {return OneMeter() / 1000.0;}
    static double constexpr OneCentimeter() {return OneMeter() / 100.0;}
    static double constexpr DiameterOfEarth() {return 12742.0 * OneKilometer();} // approximate
    static double constexpr CircumferenceOfEarth() {return 40075.0 * OneKilometer();} // approximate
    };

//=======================================================================================
//! A DRange3d that holds min/max values for an object in each of x,y,z in some coordinate system.
//! @note A BoundingBox3d makes no guarantee that the box is the minimum (smallest) box possible, just that no portion of the object
//! described by it will extend beyond its values.
// @bsiclass                                                    Keith.Bentley   03/14
//=======================================================================================
struct BoundingBox3d : DRange3d
{
  BoundingBox3d() { DRange3d::Init(); }
  explicit BoundingBox3d(DRange2dCR range2d) { DRange3d::InitFrom(&range2d.low, 2, 0.0); }
  bool IsValid() const { return !IsEmpty(); }
  void ToJson(JsonValueR value) const { JsonUtils::DRange3dToJson(value, *this); }
  void FromJson(JsonValueCR value) { JsonUtils::DRange3dFromJson(*this, value); }
};

//=======================================================================================
//! A BoundingBox3d that is aligned with the axes of a CoordinateSpace.
// @bsiclass                                                    Keith.Bentley   03/14
//=======================================================================================
struct AxisAlignedBox3d : BoundingBox3d
{
  AxisAlignedBox3d() {}
  explicit AxisAlignedBox3d(DRange3dCR range) { DRange3d::InitFrom(range.low, range.high); }
  explicit AxisAlignedBox3d(DRange2dCR range2d) { DRange3d::InitFrom(&range2d.low, 2, 0.0); }
  AxisAlignedBox3d(DPoint3dCR lowPt, DPoint3dCR highPt) { DRange3d::InitFrom(lowPt, highPt); }
  DPoint3d GetCenter() const { return DPoint3d::FromInterpolate(low, .5, high); }
};

//=======================================================================================
//! A BoundingBox3d that is aligned with the local coordinate system of a DgnElement.
// @bsiclass                                                    Keith.Bentley   03/14
//=======================================================================================
struct ElementAlignedBox3d : BoundingBox3d
{
  ElementAlignedBox3d() {}
  explicit ElementAlignedBox3d(DRange2dCR range2d) { DRange3d::InitFrom(&range2d.low, 2, 0.0); }
  ElementAlignedBox3d(double left, double front, double bottom, double right, double back, double top) { DRange3d::InitFrom(left, front, bottom, right, back, top); }
  explicit ElementAlignedBox3d(DRange3dCR range) { DRange3d::InitFrom(range.low, range.high); }

  double GetLeft() const { return low.x; }
  double GetBottom() const { return low.y; }
  double GetFront() const { return low.z; }
  double GetRight() const { return high.x; }
  double GetTop() const { return high.y; }
  double GetBack() const { return high.z; }
  double GetWidth() const { return XLength(); }
  double GetDepth() const { return YLength(); }
  double GetHeight() const { return ZLength(); }
  void SetLeft(double left) { low.x = left; }
  void SetFront(double front) { low.y = front; }
  void SetBottom(double bottom) { low.z = bottom; }
  void SetRight(double right) { high.x = right; }
  void SetBack(double back) { high.y = back; }
  void SetTop(double top) { high.z = top; }
};

//=======================================================================================
//! A DRange2d that holds min/max values for an object in each of x and y in some coordinate system.
//! @note A BoundingBox2d makes no guarantee that the box is the minimum (smallest) box possible, just that no portion of the object
//! described by it will extend beyond its values.
// @bsiclass                                                    Keith.Bentley   03/14
//=======================================================================================
struct BoundingBox2d : DRange2d
{
  BoundingBox2d() { DRange2d::Init(); }
  bool IsValid() const { return !IsEmpty(); }
};

//=======================================================================================
//! A BoundingBox2d that is aligned with the axes of a CoordinateSpace.
// @bsiclass                                                    Keith.Bentley   03/14
//=======================================================================================
struct AxisAlignedBox2d : BoundingBox2d
{
  AxisAlignedBox2d() {}
  AxisAlignedBox2d(DRange2dCR range) { DRange2d::InitFrom(range.low, range.high); }
  AxisAlignedBox2d(DPoint2dCR low, DPoint2dCR high) { DRange2d::InitFrom(low, high); }
};

//=======================================================================================
//! A BoundingBox2d that is aligned with the local coordinate system of a DgnElement.
// @bsiclass                                                    Keith.Bentley   03/14
//=======================================================================================
struct ElementAlignedBox2d : BoundingBox2d
{
  ElementAlignedBox2d() {}
  ElementAlignedBox2d(double left, double bottom, double right, double top) { DRange2d::InitFrom(left, bottom, right, top); }

  double GetLeft() const { return low.x; }
  double GetBottom() const { return low.y; }
  double GetRight() const { return high.x; }
  double GetTop() const { return high.y; }
  double GetWidth() const { return XLength(); }
  double GetHeight() const { return YLength(); }
  double GetAspectRatio() const { return XLength() / YLength(); }
  void SetLeft(double left) { low.x = left; }
  void SetBottom(double bottom) { low.y = bottom; }
  void SetRight(double right) { high.x = right; }
  void SetTop(double top) { high.y = top; }
};

//=======================================================================================
//! The position, orientation, and size of a 3d element.
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
struct Placement3d
{
protected:
  DPoint3d m_origin;
  YawPitchRollAngles m_angles;
  ElementAlignedBox3d m_boundingBox;

public:
  BE_JSON_NAME(origin)
  BE_JSON_NAME(angles)
  BE_JSON_NAME(bbox)

  Placement3d() : m_origin(DPoint3d::FromZero())
  {
  }
  Placement3d(DPoint3dCR origin, YawPitchRollAngles angles, ElementAlignedBox3dCR box = ElementAlignedBox3d()) : m_origin(origin), m_angles(angles), m_boundingBox(box) {}
  Placement3d(Placement3d const &rhs) : m_origin(rhs.m_origin), m_angles(rhs.m_angles), m_boundingBox(rhs.m_boundingBox) {}
  Placement3d(Placement3d &&rhs) : m_origin(rhs.m_origin), m_angles(rhs.m_angles), m_boundingBox(rhs.m_boundingBox) {}
  Placement3d &operator=(Placement3d &&rhs)
  {
    m_origin = rhs.m_origin;
    m_angles = rhs.m_angles;
    m_boundingBox = rhs.m_boundingBox;
    return *this;
  }
  Placement3d &operator=(Placement3d const &rhs)
  {
    m_origin = rhs.m_origin;
    m_angles = rhs.m_angles;
    m_boundingBox = rhs.m_boundingBox;
    return *this;
  }

  //! Get the origin of this Placement3d.
  DPoint3dCR GetOrigin() const { return m_origin; }

  //! Get a writable reference to the origin of this Placement3d.
  DPoint3dR GetOriginR() { return m_origin; }
  void SetOrigin(DPoint3dCR origin) { m_origin = origin; }

  //! Get the YawPitchRollAngles of this Placement3d.
  YawPitchRollAnglesCR GetAngles() const { return m_angles; }

  //! Get a writable reference to the YawPitchRollAngles of this Placement3d.
  YawPitchRollAnglesR GetAnglesR() { return m_angles; }
  void SetAngles(YawPitchRollAnglesCR angles) { m_angles = angles; }

  //! Get the ElementAlignedBox3d of this Placement3d.
  ElementAlignedBox3d const &GetElementBox() const { return m_boundingBox; }

  //! Get a writable reference to the ElementAlignedBox3d of this Placement3d.
  ElementAlignedBox3d &GetElementBoxR() { return m_boundingBox; }
  void SetElementBox(ElementAlignedBox3d const &box) { m_boundingBox = box; }

  //! Convert the origin and YawPitchRollAngles of this Placement3d into a Transform.
  Transform GetTransform() const { return m_angles.ToTransform(m_origin); }

  //! Calculate the AxisAlignedBox3d of this Placement3d.
  GEOMDLLIMPEXP AxisAlignedBox3d CalculateRange() const;

  GEOMDLLIMPEXP Json::Value ToJson() const;
  GEOMDLLIMPEXP void FromJson(JsonValueCR);

  //! Modify the origin and angles of this Placement3d by applying the specified transform.
  //! @param trans The transform to apply
  //! @return false if the operation failed
  GEOMDLLIMPEXP bool TryApplyTransform(TransformCR trans);

  //! Determine whether this Placement3d is valid.
  bool IsValid() const
  {
    if (!m_boundingBox.IsValid())
      return false;

    double maxCoord = PlacementOnEarth::CircumferenceOfEarth();

    if (m_boundingBox.low.x < -maxCoord || m_boundingBox.low.y < -maxCoord || m_boundingBox.low.z < -maxCoord ||
        m_boundingBox.high.x > maxCoord || m_boundingBox.high.y > maxCoord || m_boundingBox.high.z > maxCoord)
      return false;

    if (fabs(m_origin.x) > maxCoord || fabs(m_origin.y) > maxCoord || fabs(m_origin.z) > maxCoord)
      return false;

    return true;
  }

  GEOMDLLIMPEXP static bool IsMinimumRange(FPoint3dCR low, FPoint3dCR high, bool is2d); //!< @private
  GEOMDLLIMPEXP static bool IsMinimumRange(DPoint3dCR low, DPoint3dCR high, bool is2d); //!< @private
};

//=======================================================================================
//! The position, rotation angle, and bounding box for a 2-dimensional element.
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
struct Placement2d
{
protected:
  DPoint2d m_origin;
  AngleInDegrees m_angle;
  ElementAlignedBox2d m_boundingBox;

public:
  BE_JSON_NAME(origin)
  BE_JSON_NAME(angle)
  BE_JSON_NAME(bbox)

  Placement2d() : m_origin(DPoint2d::FromZero())
  {
  }
  Placement2d(DPoint2dCR origin, AngleInDegrees const &angle, ElementAlignedBox2dCR box = ElementAlignedBox2d()) : m_origin(origin), m_angle(angle), m_boundingBox(box) {}
  Placement2d(Placement2d const &rhs) : m_origin(rhs.m_origin), m_angle(rhs.m_angle), m_boundingBox(rhs.m_boundingBox) {}
  Placement2d(Placement2d &&rhs) : m_origin(rhs.m_origin), m_angle(rhs.m_angle), m_boundingBox(rhs.m_boundingBox) {}
  Placement2d &operator=(Placement2d &&rhs)
  {
    m_origin = rhs.m_origin;
    m_angle = rhs.m_angle;
    m_boundingBox = rhs.m_boundingBox;
    return *this;
  }
  Placement2d &operator=(Placement2d const &rhs)
  {
    m_origin = rhs.m_origin;
    m_angle = rhs.m_angle;
    m_boundingBox = rhs.m_boundingBox;
    return *this;
  }

  //! Get the origin of this Placement2d.
  DPoint2dCR GetOrigin() const { return m_origin; }
  void SetOrigin(DPoint2dCR origin) { m_origin = origin; }

  //! Get a writable reference to the origin of this Placement2d.
  DPoint2dR GetOriginR() { return m_origin; }

  //! Get the angle of this Placement2d
  AngleInDegrees GetAngle() const { return m_angle; }
  void SetAngle(AngleInDegrees const &angle) { m_angle = angle; }

  //! Get a writable reference to the angle of this Placement2d.
  AngleInDegrees &GetAngleR() { return m_angle; }

  //! Get the ElementAlignedBox2d of this Placement2d.
  ElementAlignedBox2d const &GetElementBox() const { return m_boundingBox; }

  //! Get a writable reference to the ElementAlignedBox2d of this Placement2d.
  ElementAlignedBox2d &GetElementBoxR() { return m_boundingBox; }
  void SetElementBox(ElementAlignedBox2dCR box) { m_boundingBox = box; }

  //! Convert the origin and angle of this Placement2d into a Transform.
  Transform GetTransform() const
  {
    Transform t;
    t.InitFromOriginAngleAndLengths(m_origin, m_angle.Radians(), 1.0, 1.0);
    return t;
  }

  //! Calculate an AxisAlignedBox3d for this Placement2d.
  //! @note the z values are set to +-1m
  GEOMDLLIMPEXP AxisAlignedBox3d CalculateRange() const;

  GEOMDLLIMPEXP Json::Value ToJson() const;
  GEOMDLLIMPEXP void FromJson(JsonValueCR);

  //! Determine whether this Placement2d is valid
  bool IsValid() const
  {
    if (!m_boundingBox.IsValid())
      return false;

    double maxCoord = PlacementOnEarth::CircumferenceOfEarth();

    if (m_boundingBox.low.x < -maxCoord || m_boundingBox.low.y < -maxCoord ||
        m_boundingBox.high.x > maxCoord || m_boundingBox.high.y > maxCoord)
      return false;

    if (fabs(m_origin.x) > maxCoord || fabs(m_origin.y) > maxCoord)
      return false;

    return true;
  }
};

END_BENTLEY_GEOMETRY_NAMESPACE
