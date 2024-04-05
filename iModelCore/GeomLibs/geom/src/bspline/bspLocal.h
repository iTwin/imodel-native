/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

/* DO NOT EDIT!  THIS FILE IS GENERATED. */

#pragma once


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     rotMatrix_orthogonalFromZRow (RotMatrixP rotMatrixP, DVec3dCP normalP);

Public StatusInt     SegmentSegmentClosestApproachPoints
(
DPoint3dR approach1,    // point on first segment
DPoint3dR approach2,    // point on second segment
DPoint3dCR    pt1,         /* => vector 1 */
DPoint3dCR    dir1,
DPoint3dCR    pt2,         /* => vector 2 */
DPoint3dCR    dir2,
double testDistance = DBL_MAX  // call it an error if points are farther apart than this distance
);





