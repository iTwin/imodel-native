/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! @description Generate 0, 1, or 2 conics for the locii of tangent circle centers.
//! @param pConicArray OUT array of conics --- hyperbola, ellipse depending on relative size and position
//!        of circles.  Caller MUST dimension this array to at least 2.
//! @param pNumConic OUT number of conics placed in output bufferr.
//! @param pCenterA IN center of first circle.
//! @param rA IN radius of first circle
//! @param pCenterB IN center of second circle.
//! @param rB IN radius of second circle.
//! @group "Tangent Circles"
//!
Public GEOMDLLIMPEXP void bsiGeom_circleTTLociiOfCenters
(
DConic4dP pConicArray,
int      *pNumConic,
DPoint3dP pCenterA,
double   rA,
DPoint3dP pCenterB,
double   rB
);
//! @description return a single conic that is the locii of centers of circles
//! tangent to two circles of given signed radius.
Public GEOMDLLIMPEXP bool bsiDConic4d_initSignedCircleTangentCenters
(
DConic4dP pConic,
DPoint3dP pCenterA,
double rA,
DPoint3dP pCenterB,
double rB
);
//!
//! @description compute the centers of circles which are tangent to 3 given circles.
//!    Only xy parts of the inputs participate in the calculation.
//! @param pCenterArrayOut OUT array of centers of tangent circles.
//! @param pRadiusArrayOut OUT array of radii of tangent circls
//! @param pNumOut OUT number of tangent circles
//! @param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
//!        possible.
//! @param pCenterArrayIn IN array of 3 known circle centers.
//! @param pRadiusArrayIn IN array of 3 known radii.  Zero radius inputs are
//!        permitted.
//! @group "Tangent Circles"
//!
Public GEOMDLLIMPEXP void bsiGeom_circleTTTCircleConstruction
(
DPoint3dP pCenterArrayOut,
double   *pRadiusArrayOut,
int      *pNumOut,
int      maxOut,
DPoint3dP pCenterArrayIn,
double   *pRadiusArrayIn
);

//!
//! @description compute the centers of circles which are tangent to 2 given circles and a line
//!    Only xy parts of the inputs participate in the calculation.
//! @param pCenterArrayOut OUT array of centers of tangent circles.
//! @param pRadiusArrayOut OUT array of radii of tangent circls
//! @param pNumOut OUT number of tangent circles
//! @param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
//!        possible.
//! @param pCenterArrayIn IN array of 2 known circle centers.
//! @param pRadiusArrayIn IN array of 2 known radii.  Zero radius inputs are permitted.
//! @param pLinePoint IN any point on the line.
//! @param pLineDirection IN a vector in the direction of the line.
//! @group "Tangent Circles"
//!
Public GEOMDLLIMPEXP void bsiGeom_circleTTTCircleCircleLineConstruction
(
DPoint3dP pCenterArrayOut,
double   *pRadiusArrayOut,
int      *pNumOut,
int      maxOut,
DPoint3dP pCenterArrayIn,
double   *pRadiusArrayIn,
DPoint3dP pLinePoint,
DVec3dP pLineDirection
);

#ifdef __cplusplus

//!@description compute the centers of circles which are tangent to 2 given circles and a line
//!   Only xy parts of the inputs participate in the calculation.
//!@param pCenterArrayOut OUT array of centers of tangent circles.
//!@param pRadiusArrayOut OUT array of radii of tangent circls
//!@param pTangentAOut OUT array of tangency points on circle A
//!@param pTangentBOut OUT array of tangency points on circle B
//!@param pTangentCOut OUT array of tangency points on line C
//!@param pNumOut OUT number of tangent circles
//!@param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
//!        possible.
//! @param [in] centerA center of first circle
//! @param [in] radiusA radius of first circle
//! @param [in] centerB center of second circle
//! @param [in] radiusB radius of second circle
//! @param [in] linePointC point on line
//! @param [in] lineDirectionC direction of line.
Public GEOMDLLIMPEXP void bsiGeom_circleTTTCircleCircleLineConstructionExt
(
DPoint3dP   pCenterArrayOut,
double   *  pRadiusArrayOut,
DPoint3dP   pTangentAOut,
DPoint3dP   pTangentBOut,
DPoint3dP   pTangentCOut,
int         &numOut,
int         maxOut,
DPoint3dCR  centerA,
double      radiusA,
DPoint3dCR  centerB,
double      radiusB,
DPoint3dCR  linePointC,
DVec3dCR    lineDirectionC
);

//!@description compute the centers of circles which are tangent to 3 given circles
//!   Only xy parts of the inputs participate in the calculation.
//!@param pCenterArrayOut OUT array of centers of tangent circles.
//!@param pRadiusArrayOut OUT array of radii of tangent circls
//!@param pTangentAOut OUT array of tangency points on circle A
//!@param pTangentBOut OUT array of tangency points on circle B
//!@param pTangentCOut OUT array of tangency points on circle C
//!@param pNumOut OUT number of tangent circles
//!@param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
//!        possible.
//! @param [in] centerA center of first circle
//! @param [in] radiusA radius of first circle
//! @param [in] centerB center of second circle
//! @param [in] radiusB radius of second circle
//! @param [in] centerC center of second circle
//! @param [in] radiusC radius of second circle
Public GEOMDLLIMPEXP void bsiGeom_circleTTTCircleCircleCircleConstruction
(
DPoint3dP   pCenterArrayOut,
double   *  pRadiusArrayOut,
DPoint3dP   pTangentAOut,
DPoint3dP   pTangentBOut,
DPoint3dP   pTangentCOut,
int         &numOut,
int         maxOut,
DPoint3dCR  centerA,
double      radiusA,
DPoint3dCR  centerB,
double      radiusB,
DPoint3dCR  centerC,
double      radiusC
);


/*---------------------------------------------------------------------------------**//**
* @description compute the centers of circles which are tangent to a circle and 2 given lines
*    Only xy parts of the inputs participate in the calculation.
* @param pCenterArrayOut OUT array of centers of tangent circles.
* @param pRadiusArrayOut OUT array of radii of tangent circls
* @param pNumOut OUT number of tangent circles
* @param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
        possible.
* @param pCenterIn IN known circle center centers.
* @param pRadiusIn IN circle radius.  Zero radius inputs are permitted.
* @param pLinePointA IN any point on line A.
* @param pLineDirectionA IN a vector in the direction of line A.
* @param pLinePointB IN any point on line B
* @param pLineDirectionB IN a vector in the direction of line B.
* @group "Tangent Circles"
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_circleTTTLineLineCircleConstruction
(
DPoint3dP pCenterArrayOut,
double   *pRadiusArrayOut,
DPoint3dP pTangentPointAOut,
DPoint3dP pTangentPointBOut,
DPoint3dP pTangentPointCOut,
int      &numOut,
int      maxOut,
DPoint3dCR linePointA,
DVec3dCR   lineDirectionA,
DPoint3dCR linePointB,
DVec3dCR   lineDirectionB,
DPoint3dCR centerC,
double     radiusC
);

//! @description compute the centers of circles which are tangent to 3 lines.
//!   Only xy parts of the inputs participate in the calculation.
//! @param pCenterArrayOut OUT array of centers of tangent circles.
//! @param pRadiusArrayOut OUT array of radii of tangent circls
//! @param pNumOut OUT number of tangent circles
//! @param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
//!      possible.
//! @param pCenterIn IN known circle center centers.
//! @param pRadiusIn IN circle radius.  Zero radius inputs are permitted.
//! @param pLinePointA IN any point on line A.
//! @param pLineDirectionA IN a vector in the direction of line A.
//! @param pLinePointB IN any point on line B
//! @param pLineDirectionB IN a vector in the direction of line B.
//! @param pLinePointC IN any point on line C
//! @param pLineDirectionC IN a vector in the direction of line C.
Public GEOMDLLIMPEXP void bsiGeom_circleTTTLineLineLineConstruction
(
DPoint3dP pCenterArrayOut,
double   *pRadiusArrayOut,
DPoint3dP pTangentPointAOut,
DPoint3dP pTangentPointBOut,
DPoint3dP pTangentPointCOut,
int      &numOut,
int      maxOut,
DPoint3dCR linePointA,
DVec3dCR   lineDirectionA,
DPoint3dCR linePointB,
DVec3dCR   lineDirectionB,
DPoint3dCR linePointC,
DVec3dR    lineDirectionC
);

//!@description compute the centers of circles which are tangent to a line and a circle, and have given radius
//!   Only xy parts of the inputs participate in the calculation.
//!@param pCenterArrayOut OUT array of centers of tangent circles.
//!@param pTangentPointAOut OUT array of tangencies on line A.
//!@param pTangentPointBOut OUt array of tangencies on circle B.
//!@param pNumOut OUT number of tangent circles
//!@param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
//!      possible.
//!@param pCenterIn IN known circle center centers.
//!@param pRadiusIn IN circle radius.  Zero radius inputs are permitted.
//!@param linePointA IN any point on line A.
//!@param lineDirectionA IN a vector in the direction of line A.
//!@param circleCenterB IN center of circle B
//!@param radiusB IN radius of circle B
//!@param radiusC IN target circle radius.
Public GEOMDLLIMPEXP void bsiGeom_circleTTLineCircleConstruction
(
DPoint3dP pCenterArrayOut,
DPoint3dP pTangentPointAOut,
DPoint3dP pTangentPointBOut,
int      &numOut,
int      maxOut,
DPoint3dCR linePointA,
DVec3dCR   lineDirectionA,
DPoint3dCR circleCenterB,
double     radiusB,
double     radiusC
);

//!@description compute the centers of circles which are tangent to a line and a circle, and have given radius
//!   Only xy parts of the inputs participate in the calculation.
//!@param pCenterArrayOut OUT array of centers of tangent circles.
//!@param pTangentPointAOut OUT array of tangencies on line A.
//!@param pTangentPointBOut OUt array of tangencies on circle B.
//!@param pNumOut OUT number of tangent circles
//!@param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
//!      possible.
//!@param pCenterIn IN known circle center centers.
//!@param pRadiusIn IN circle radius.  Zero radius inputs are permitted.
//!@param centerA IN center of circle A
//!@param radiusA IN radius of circle A
//!@param centerB IN center of circle B
//!@param radiusB IN radius of circle B
//!@param radiusC IN target circle radius.
Public GEOMDLLIMPEXP void bsiGeom_circleTTCircleCircleConstruction
(
DPoint3dP pCenterArrayOut,
DPoint3dP pTangentPointAOut,
DPoint3dP pTangentPointBOut,
int      &numOut,
int      maxOut,
DPoint3dCR centerA,
double     radiusA,
DPoint3dCR centerB,
double     radiusB,
double     radiusC
);

#endif
END_BENTLEY_GEOMETRY_NAMESPACE
