/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Evaluate XT*sigma*Y for sigma=diag(1,1,1,-1)
//!
Public GEOMDLLIMPEXP double     bsiQuadric_sphereProduct
(
DPoint4dCP pX,
DPoint4dCP pY
);

//!
//! Evaluate XT*sigma*Y for sigma=diag(1,1,0,-r^2)
//!
Public GEOMDLLIMPEXP double     bsiQuadric_circleProduct
(
DPoint4dCP pX,
DPoint4dCP pY,
double    r
);

//!
//! Evaluate XT*sigma*Y for sigma=diag(1,1,0,-1)
//!
Public GEOMDLLIMPEXP double     bsiQuadric_cylinderProduct
(
DPoint4dCP pX,
DPoint4dCP pY
);

//!
//! Evaluate XT*sigma*Y for sigma=diag(1,1,-1,0)
//!
Public GEOMDLLIMPEXP double     bsiQuadric_coneProduct
(
DPoint4dCP pX,
DPoint4dCP pY
);

//!
//!
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_cylinderToCartesian
(
DPoint4dP pXYZ,
DPoint4dCP pR
);

//!
//! Evaluate (homogeneous) partial derivatives for a unit cone along the z axis.
//!
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_conePartials
(
DPoint4dP pPartial0,
DPoint4dP pPartial1,
DPoint4dP pPartial2,
DPoint4dP pPartial3,
DPoint4dCP pR
);

//!
//! Evaluate (homogeneous) partial derivatives for a unit cylinder along
//! the z axis.
//!
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_cylinderPartials
(
DPoint4dP pPartial0,
DPoint4dP pPartial1,
DPoint4dP pPartial2,
DPoint4dP pPartial3,
DPoint4dCP pR
);

//!
//! Evaluate (homogeneous) partial derivatives for a unit sphere.
//!
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_spherePartials
(
DPoint4dP pPartial0,
DPoint4dP pPartial1,
DPoint4dP pPartial2,
DPoint4dP pPartial3,
DPoint4dCP pR
);

//!
//! Evaluate (homogeneous) partial derivatives for torus with primary
//! radius 1 in the xy plane, secondary radius b.
//!
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_torusPartials
(
DPoint4dP pPartial0,
DPoint4dP pPartial1,
DPoint4dP pPartial2,
DPoint4dP pPartial3,
DPoint4dCP pR,
double    b
);

//!
//! Evaluate the (homogeneous) point on a torus, given latitude, longitude,
//!    radius (from major hoop, multiple of minor hoop radius).
//!
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_torusToCartesian
(
DPoint4dP pXYZ,
DPoint4dCP pR,
double    hoopRadius
);

//!
//! Convert (theta, phi, r, w) to (w*r*cos*COS, w*r*sin*COS, w*r*SIN, w)
//!
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_sphereToCartesian
(
DPoint4dP pXYZ,
DPoint4dCP pR
);

//!
//! Convert (r*w*cos, r*w*sin, w*z, w) to  (theta, z, r, 1)
//!
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_cartesianToCylinder
(
DPoint4dP pR,
DPoint4dCP pXYZ
);

//!
//! Convert (xyzw)=(r*w*c*C, r*w*s*C, r*w*S, w) to  (theta, phi, r, 1)
//!
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_cartesianToSphere
(
DPoint4dP pR,
DPoint4dCP pXYZ
);

//!
//! Convert
//!       (wC(1+rho*r*s), wS(1+rho*r*c),w*rho*r*s,w)
//!   to
//!       (theta, phi, rho, w)
//!
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_cartesianToTorus
(
DPoint4dP pR,
DPoint4dCP pXYZ,
double     hoopRadius      /* Assumed nonzero.  Assumed positive */
);

//!
//! Convert (r*w*z*cos, r*w*z*sin, w*z, w) to  (theta, z, r, 1)
//!
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_cartesianToCone
(
DPoint4dP pR,
DPoint4dCP pXYZ
);

//!
//! Convert (theta, z, r, w) to (r*w*z*cos, r*w*z*sin, w*z, w)
//!
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_coneToCartesian
(
DPoint4dP pXYZ,
DPoint4dCP pR
);

//!
//!
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_diskToCartesian
(
DPoint4dP pXYZ,
DPoint4dCP pR
);

//!
//!
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_diskPartials
(
DPoint4dP pPartial0,
DPoint4dP pPartial1,
DPoint4dP pPartial2,
DPoint4dP pPartial3,
DPoint4dCP pR
);

//!
//!
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_cartesianToDisk
(
DPoint4dP pR,
DPoint4dCP pXYZ
);

END_BENTLEY_GEOMETRY_NAMESPACE

