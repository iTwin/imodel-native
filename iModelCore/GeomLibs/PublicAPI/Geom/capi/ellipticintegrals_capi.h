/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! @description Return arc length of ellipse given in major/minor axis form.
//! @param a IN major axis of ellipse.
//! @param b IN minor axis of ellipse.
//! @param startRadians IN start of arc.
//! @param sweepRadians IN sweep angle of arc.
//! @return arc length
//! @group "Elliptic Integrals"
//!
Public GEOMDLLIMPEXP double bsiGeom_ellipseArcLength
(
double a,
double b,
double startRadians,
double sweepRadians
);

//!
//! @description Return sweep angle which travels specified arc length of ellipse given in major/minor axis form.
//! @param a IN major axis of ellipse
//! @param b IN minor axis of ellipse
//! @param startRadians IN start of arc
//! @param distance IN arc length to travel
//! @return sweep angle
//! @group "Elliptic Integrals"
//!
Public GEOMDLLIMPEXP double bsiGeom_ellipseInverseArcLength
(
double a,
double b,
double startRadians,
double distance
);

//!
//! @description Return radius of curvature on simple major/minor ellipse.
//! @param a IN major axis of ellipse
//! @param b IN minor axis of ellipse
//! @param radians IN angular parameter for evaluation.
//! @return radius of curvature
//! @group "Elliptic Integrals"
//!
Public GEOMDLLIMPEXP double bsiGeom_ellipseRadiusOfCurvature
(
double a,
double b,
double radians
);

//!
//! @description Return parameter angle at which a major/minor ellipse has specified radius of curvature.
//! @param pRadians OUT first quadrant parameter angle.
//! @param a IN major axis of ellipse
//! @param b IN minor axis of ellipse
//! @param rho IN target radius.  The valid range is between a^2/b and b^2/a.
//! @return true if the radius of curvature is valid.
//! @group "Elliptic Integrals"
//!
Public GEOMDLLIMPEXP bool    bsiGeom_ellipseInverseRadiusOfCurvature (double *pRadians, double a, double b, double rho);

END_BENTLEY_GEOMETRY_NAMESPACE

