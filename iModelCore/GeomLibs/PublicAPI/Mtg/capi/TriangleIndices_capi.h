/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
*
* Install user-supplied buffer pointers and size limits.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void jmdlTriangleIndices_init
(
TriangleIndices   *pMesh,
DPoint3d        *pPointArray,
DPoint3d        *pNormalArray,
DPoint2d        *pUVArray,
int             *pTriangleToVertexArray,
int             *pTriangleToTriangleArray,
int             maxPoint,
int             maxTri
);

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void jmdlTriangleIndices_printIndices
(
TriangleIndices   *pMesh
);

/*---------------------------------------------------------------------------------**//**
*
* @param numPoint IN number of points around convex polygon
* @param startIndex IN index of first point of triangulation.  Triangulation
*           sweeps away from this point.
* @return
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlTriangleIndices_triangulateConvex
(
TriangleIndices   *pMesh,
int             numPoint,
int             startIndex
);

END_BENTLEY_GEOMETRY_NAMESPACE

