/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_newContext                                       |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP RIMSBS_Context *jmdlRIMSBS_newContext
(
void
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_freeContext                                      |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     jmdlRIMSBS_freeContext
(
RIMSBS_Context *pContext
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_initContext                                      |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     jmdlRIMSBS_initContext
(
RIMSBS_Context *pContext
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_setupRGCallbacks                                 |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     jmdlRIMSBS_setupRGCallbacks
(
RIMSBS_Context *pContext,
RG_Header       *pRG
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_releaseMem                                       |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     jmdlRIMSBS_releaseMem
(
RIMSBS_Context *pContext
);


/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getUserPointer                                   |
|                                                                       |
|                                                                       |
| Get the user data pointer part of a curve.                            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     jmdlRIMSBS_getUserPointer
(
RIMSBS_Context  *pContext,
void            **pUserData,
int             index
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getUserInt                                       |
|                                                                       |
|                                                                       |
| Get the user int part of a curve.                                     |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     jmdlRIMSBS_getUserInt
(
RIMSBS_Context  *pContext,
int             *pUserInt,
int             index
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getUserInt64                                     |
|                                                                       |
|                                                                       |
| Get the user Int64 part of a curve.                                   |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     jmdlRIMSBS_getUserInt64
(
RIMSBS_Context  *pContext,
int64_t         *pUserInt64,
int             index
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_setUserInt                                       |
|                                                                       |
|                                                                       |
| Set the user int part of a curve.                                     |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     jmdlRIMSBS_setUserInt
(
RIMSBS_Context  *pContext,
int             index,
int             userInt
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_setUserInt64                                     |
|                                                                       |
|                                                                       |
| Set the user Int64 part of a curve.                                   |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     jmdlRIMSBS_setUserInt64
(
RIMSBS_Context  *pContext,
int             index,
int64_t         userInt64
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_setUserPointer                                   |
|                                                                       |
|                                                                       |
| Set the user pointer part of a curve.                                 |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     jmdlRIMSBS_setUserPointer
(
RIMSBS_Context  *pContext,
int             index,
void            *pUserData
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_setGroupId                                       |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_setGroupId
(
RIMSBS_Context  *pContext,
RIMSBS_CurveId  curveId,
int             groupId
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_setGroupId                                       |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_getGroupId
(
RIMSBS_Context  *pContext,
int             *pGroupId,
RIMSBS_CurveId  curveId
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_setCurrGroupId                                   |
|                                                                       |
|                                                                       |
| Set the current group id to be applied to new elements.               |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_setCurrGroupId
(
RIMSBS_Context  *pContext,
int             groupId
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getImmediateParent                               |
|                                                                       |
|                                                                       |
| Get the immediate parent of a curve.                                  |
| Returns the same index if not a child.                                |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     jmdlRIMSBS_getImmediateParent
(
RIMSBS_Context  *pContext,
int             *pParentIndex,
int             index
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getFarthestParent                                |
|                                                                       |
|                                                                       |
| Follow parent pointers as far as possible.                            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     jmdlRIMSBS_getFarthestParent
(
RIMSBS_Context  *pContext,
int             *pParentIndex,
int             index
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_addArc                                           |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      jmdlRIMSBS_addArc
(
RIMSBS_Context  *pContext,
int             userInt,
void            *pUserData,
DMatrix3d       *pMatrix,
DPoint3d        *pCenter,
double          r0,
double          r1,
double          theta0,
double          sweep
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_addDEllipse3d                                    |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      jmdlRIMSBS_addDEllipse3d
(
RIMSBS_Context  *pContext,
int             userInt,
void            *pUserData,
const DEllipse3d *pEllipse
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_addDataCarrier                                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      jmdlRIMSBS_addDataCarrier
(
RIMSBS_Context  *pContext,
int             userInt,
void            *pUserData
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_addCurveChain                                    |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      jmdlRIMSBS_addCurveChain
(
RIMSBS_Context  *pContext,
int             userInt,
void            *pUserData,
RIMSBS_CurveId  primaryCurveId
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_addCurveToCurveChain                             |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_addCurveToCurveChain
(
RIMSBS_Context  *pContext,
RIMSBS_CurveId  chainId,
RIMSBS_CurveId  childCurveId
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_isCurveChain                                     |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_isCurveChain
(
RIMSBS_Context  *pContext,
RIMSBS_CurveId *pPrimaryCurveId,
RIMSBS_CurveId  curveId
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getChildCurve                                    |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_getCurveChainChild
(
RIMSBS_Context  *pContext,
RIMSBS_CurveId  curveId,
int             childIndex,
RIMSBS_CurveId  *pChildId
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_createAlternateMSBsplineCurve                    |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      jmdlRIMSBS_createAlternateMSBsplineCurve
(
RIMSBS_Context  *pContext,
RIMSBS_CurveId  parentId
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_addMSBsplineCurve                                |
|                                                                       |
|                                                                       |
| Copy the curve structure (bitwise) into the context.  Caller must     |
| NOT free the curve.  Curve may be subdivided at the whim of RIMSBS.   |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      jmdlRIMSBS_addMSBsplineCurve
(
RIMSBS_Context  *pContext,
int             userInt,
void            *pUserData,
MSBsplineCurveP pCurve
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_addMSBsplineCurve                                |
|                                                                       |
|                                                                       |
| Copy the curve structure (bitwise) into the context.  Caller must     |
| NOT free the curve. Curve is never subidivided.                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      jmdlRIMSBS_addMSBsplineCurveDirect
(
RIMSBS_Context  *pContext,
int             userInt,
void            *pUserData,
MSBsplineCurveP pCurve
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_addBezier                                        |
|                                                                       |
|                                                                       |
| Create an MSBsplineCurve for the given bezier.                        |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      jmdlRIMSBS_addBezier
(
RIMSBS_Context  *pContext,
int             userInt,
void            *pUserData,
const DPoint4d  *pPoleArray,
int             order
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_createSubcurve                                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_createSubcurve
(
RIMSBS_Context  *pContext,
RIMSBS_CurveId  *pNewCurveId,
RIMSBS_CurveId  parentCurveId,
double          s0,
double          s1
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_createSubcurveExt                                |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_createSubcurveExt
(
RIMSBS_Context  *pContext,
RIMSBS_CurveId  *pNewCurveId,
RIMSBS_CurveId  parentCurveId,
RIMSBS_CurveId  partialCurveId,
double          s0,
double          s1
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_consolidateCoincidentGeometry                    |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_consolidateCoincidentGeometry
(
RIMSBS_Context  *pContext,
double tolerance
);

END_BENTLEY_GEOMETRY_NAMESPACE

