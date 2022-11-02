/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void tristrip_init       /* initialize the tristrip buffer */
(
DPoint2d **refParamPP,
DPoint3d *refPointP,
DPoint3d *refNormalP,
int (*triangleFunction)(),
void *userDataP,
DPoint2d *scaleP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int tristrip_addTriangle         /* Put a (possibly permuted) triangle in the buffer */
(
int i0,         /* First vertex index */
int i1,         /* Second vertex index */
int i2          /* Third vertex index */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int tristrip_flush       /* Flush the tristrip buffer */
(
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspmesh_startCounters       /* Clear tristrip statistics counters */
(
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspmesh_summaryCounters
(
);

END_BENTLEY_GEOMETRY_NAMESPACE

