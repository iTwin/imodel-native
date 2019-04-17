/*-------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/
#include <Bentley/BeTimeUtilities.h>

//! @file CurveConstraints.h contains API for building curves with mixed constraints.
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
//! A CurveConstraint struct holds a single curve construction condition.
//! The structure is passed by value to callers (as return from CreateXXX methods).
//!
//! Based on inputs (e.g. from GUI) A caller will
//!<ul>
//!<li>Make calls to various static constructor methods CurveConstraint::CreateXXXXX ()
//!<li>Build up an array of those individual constrants
//!<li>Pass that array to a solver method such as
//!<ul>
//!<li>ConstrainedConstruction::ConstructLines
//!<li>ConstrainedConstruction::ConstructArcs
//!</ul>
//!</ul>
//! The following are the constraint types:
//!<ul>
//!<li> CreateThroughPoint(DPoint3d): The constructed curve must bass through the point
//!<li> CreatePointAndDirection (DPoint3d, DVec3d): The constructed curve must bass through the point with indicated direction
//!<li> CurveConstraint CreateCenter (DPoint3d): The curve (i.e.circle) must have the given center
//!<li> CreatePerpendicularNear (referenceCurve, fraction):  The constructed curve should be perpendicular to given referenceCurve near the fraction
//!<li> CreateClosestPoint referenceCurve): The constructed curve should be to a closest point on this referenceCurve
//!</ul>
struct CurveConstraint
{
//! Type code for various constraints . . .
enum class Type
    {
    None,
    ThroughPoint,
    PointAndDirection,
    Center,
    PerpendicularNear,
    ClosestPoint,
    Tangent,
    ResultFrame,
    Radius
    };
private:
Type m_type;
CurveLocationDetail m_location;
ICurvePrimitivePtr m_curve; // redundant of the CurveLocationDetail -- but as a Ptr so the lifecycle management works
DRay3d m_ray;
ValidatedTransform m_resultFrame;
ValidatedDouble m_distance;
// constructor with detail
CurveConstraint (Type type, CurveLocationDetailCR detail);
// constructor with detail and ray
CurveConstraint (Type type, CurveLocationDetailCR detail, DRay3dCR ray);
// constructor with frame
CurveConstraint (Type type, TransformCR frame);
// constructor with distance
CurveConstraint (Type type, double distance);

friend struct ConstraintMatchTable;
public:
//! Create a constraint to pass through a point.
GEOMDLLIMPEXP static CurveConstraint CreateThroughPoint (DPoint3dCR point);
//! Create a constraint to  pass through a point, and have a specified direction there
GEOMDLLIMPEXP static CurveConstraint CreatePointAndDirection (DPoint3dCR point, DVec3dCR direction);
//! Create a center point constraint.
GEOMDLLIMPEXP static CurveConstraint CreateCenter (DPoint3dCR point);
//! Create a constraint to use the closest point on a curve
GEOMDLLIMPEXP static CurveConstraint CreateClosestPoint (ICurvePrimitiveCP curve);

//! Create a constraint to use a perpendicular near a fractional position on a curve
GEOMDLLIMPEXP static CurveConstraint CreatePerpendicularNear (ICurvePrimitiveCP curve, double fraction);

//! Create a constraint to be tangent to a curve, with fraction at possible bias point
GEOMDLLIMPEXP static CurveConstraint CreateTangent (ICurvePrimitiveCP curve, double fraction);

//! Create a coordinate system for results
//!<ul>
//!<li>The XY columns of the matrix part are the plane for circle by center and radius
//!<li>The XY columns of the matrix part are the plane for circle by center and point
//!</ul>
GEOMDLLIMPEXP static CurveConstraint CreateResultFrame (TransformCR frame);

//! Create a radius constraint.
GEOMDLLIMPEXP static CurveConstraint CreateRadius (double radius);


//! return the type
GEOMDLLIMPEXP Type GetType () const;

//! Test the constraint type.
GEOMDLLIMPEXP bool IsType (Type t) const;
//! Return the constraint point.
GEOMDLLIMPEXP DPoint3d Point () const;
//! Return the constratint ray
GEOMDLLIMPEXP DRay3d PointAndDirection () const;
//! return a reference to the curve location detail ..
GEOMDLLIMPEXP CurveLocationDetailCR Location () const;
//! Return the distance value
GEOMDLLIMPEXP ValidatedDouble GetDistance () const;
//! Return the resultFrame
GEOMDLLIMPEXP ValidatedTransform GetResultFrame () const;

};

//! Static methods for apply solver logic to bvector<CurveConstraint>
struct ConstrainedConstruction
{
//! Return line segments that satisfy the constraints
GEOMDLLIMPEXP static void ConstructLines (bvector<CurveConstraint> &constraints, bvector<ICurvePrimitivePtr> &result);
//! Return arcs that satisfy the constraints
GEOMDLLIMPEXP static void ConstructCircularArcs  (bvector<CurveConstraint> &constraints, bvector<ICurvePrimitivePtr> &result);
};

END_BENTLEY_GEOMETRY_NAMESPACE
