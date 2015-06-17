/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/VecMath.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <Bentley/Bentley.h>
#include <Geom/GeomApi.h>

BEGIN_BENTLEY_NAMESPACE
/*=================================================================================**//**
* FOR COMPATIBILITY ONLY - These methods are to avoid having published mdl functions
* exported from dgnplatform. Some of these methods use bad/coarse uor tolerances and
* should not be used in any new code.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct LegacyMath
{
DGNPLATFORM_EXPORT static void LinearInterpolateQuaternion (double outQuat[4], double const p[4], double const q[4], double t, int spin);

DGNPLATFORM_EXPORT static bool RpntEqual (DPoint3dCP pt1, DPoint3dCP pt2);
DGNPLATFORM_EXPORT static bool DUorEqual (double r1, double r2);
DGNPLATFORM_EXPORT static bool DEqual (double r1, double r2);

struct Vec
    {
    DGNPLATFORM_EXPORT static bool AreParallel (DPoint3dCP Vector1, DPoint3dCP Vector2);
    DGNPLATFORM_EXPORT static bool ArePerpendicular (DPoint3dCP Vector1, DPoint3dCP Vector2);
    DGNPLATFORM_EXPORT static void ComputeRangeProjection (double *minProjectionP, double *maxProjectionP, DPoint3dP rangeLowP, DPoint3dP rangeHighP, DPoint3dP rayOriginP, DPoint3dP rayNormalP);
    DGNPLATFORM_EXPORT static void ProjectPointToPlane (DPoint3dP outPointP, DPoint3dCP inPpointP, DPoint3dCP planePointP, DPoint3dCP normalP);
    DGNPLATFORM_EXPORT static double AngleAndAxisOfRotationFromVectorToVector (DPoint3dP pAxis, DPoint3dCP pStartVector, DPoint3dCP pEndVector);
    DGNPLATFORM_EXPORT static bool ColinearToTolerance (DPoint3dCP, double);
    DGNPLATFORM_EXPORT static bool Colinear (DPoint3dCP pointArrayP);                                
    DGNPLATFORM_EXPORT static StatusInt LinePlaneIntersectParameter (DPoint3dP intersectionP, double *parameterP, DPoint3dCP lineStartP, DPoint3dCP lineDirectionP, DPoint3dCP planePointP, DPoint3dCP planeNormalP);
    DGNPLATFORM_EXPORT static int LinePlaneIntersect (DPoint3dP intersectPt, DPoint3dCP linePt, DPoint3dCP lineNormal, DPoint3dCP planePt, DPoint3dCP planeNormal, int perpendicular);
    DGNPLATFORM_EXPORT static int PlanePlaneIntersect (DPoint3dP intersectionPointP, DPoint3dP intersectionDirectionP, DPoint3dP point1P, DPoint3dP n1, DPoint3dP point2P, DPoint3dP n2);
    DGNPLATFORM_EXPORT static int ProjectPointToLine (DPoint3d *outPointP, double *outFractionP, DPoint3dCP inPointP, DPoint3dCP startPointP, DPoint3dCP endPointP);
    };

struct RMatrix
    {
    DGNPLATFORM_EXPORT static void MultiplyRange (DPoint3dP lowP, DPoint3dP highP, RotMatrixCP rotMatrixP);
    DGNPLATFORM_EXPORT static void FromNormalVector (RotMatrixP rotMatrixP, DPoint3dCP normalP);
    DGNPLATFORM_EXPORT static int FromYVector (RotMatrixP matrixP, DPoint3dCP  yNormalP);
    DGNPLATFORM_EXPORT static int FromXVector (RotMatrixP rotMatrixP, DPoint3dCP xNormalP);
    DGNPLATFORM_EXPORT static void FromVectorToVector (RotMatrixP pMatrix, DPoint3dCP pStartVector, DPoint3dCP pEndVector);
    DGNPLATFORM_EXPORT static void GetColumnScaleVector (RotMatrixP normalizedRMatrix, DPoint3dP scaleVector, RotMatrixCP in);
    };

struct TMatrix
    {
    DGNPLATFORM_EXPORT static void TransformArc (double* majorAxisLengthP, double* minorAxisLengthP, RotMatrixP matrixP, double* xAngleP, double* startRadiansP, double* sweepRadiansP, TransformCP transformP, bool threeD);
    DGNPLATFORM_EXPORT static void ComposeOrientationOriginScaleXY (TransformP pOutTrans, TransformCP pInTrans, RotMatrixCP pRotMatrix, DPoint3dCP pOrigin, double xScale, double yScale);
    DGNPLATFORM_EXPORT static TransformP ComposeLocalOriginOperations (TransformP pTransform, DPoint3dCP pP2, RotMatrixCP pR2, RotMatrixCP pR1, DPoint3dCP pP1);
    DGNPLATFORM_EXPORT static void ComposeOrientationOriginScaleXYShear (TransformP pOutTrans, TransformCP pInTrans, RotMatrixCP pRotMatrix, DPoint3dCP pOrigin, double xScale, double yScale, double slantRadians);
    DGNPLATFORM_EXPORT static void SetMatrixColumn (TransformP transformP, DPoint3dCP vectorP, int columnIndex);
    DGNPLATFORM_EXPORT static void SetMatrixRow (TransformP transformP, DPoint3dCP vectorP, int rowIndex);
    DGNPLATFORM_EXPORT static void FromNormalizedRowsOfTMatrix (TransformP normalizedTMatrix, DPoint3dP scaleVector, TransformCP in);
    DGNPLATFORM_EXPORT static StatusInt Unscale (TransformP newTransform, TransformCP oldTransform, DPoint3dCP anchor);
    };

}; // LegacyMath

END_BENTLEY_NAMESPACE
