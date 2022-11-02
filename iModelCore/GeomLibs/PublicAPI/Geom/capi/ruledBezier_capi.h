/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

GEOMDLLIMPEXP bool bsiBezier_addRuledPatchXYIntersections
(
bvector <SolidLocationDetail> &pickData,
DPoint2dCR target,
DPoint3dCP coffA,
DPoint3dCP coffB,
int order
);

GEOMDLLIMPEXP bool bsiBezier_addRuledPatchXYIntersections
(
bvector <SolidLocationDetail> &pickData,
DPoint2dCR target,
DPoint4dCP coffA,
DPoint4dCP coffB,
int order
);

GEOMDLLIMPEXP bool bsiBezier_addRayRuledSurfaceIntersections
(
bvector <SolidLocationDetail> &pickData,
DRay3dCR ray,
DPoint3dCP coffA,
DPoint3dCP coffB,
int order
);

GEOMDLLIMPEXP bool bsiBezier_addRayRuledSurfaceIntersections
(
bvector <SolidLocationDetail> &pickData,
DRay3dCR ray,
DPoint4dCP coffA,
DPoint4dCP coffB,
int order
);

GEOMDLLIMPEXP void bsiBezier_evaluateRuled
(
double u,
double v,
DPoint3dCP coffA,
DPoint3dCP coffB,
int order,
DPoint3dR xyz,
DVec3dR dXdu,
DVec3dR dXdv
);

Public bool bsiBezier_ruledPatchClosestPoint
(
SolidLocationDetail &pickData,
DPoint3dCR spacePoint,
DPoint4dCP coffA,
DPoint4dCP coffB,
int order
);

Public bool bsiDEllipse3d_ruledPatchClosestPoint
(
SolidLocationDetailR pickData,
DPoint3dCR spacePoint,
DEllipse3dCR ellipseA,
DEllipse3dCR ellipseB
);
END_BENTLEY_GEOMETRY_NAMESPACE

