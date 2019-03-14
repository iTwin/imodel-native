/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/hbasemod.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*====================================================================
**
**  hbasemod.h
**
**      This file declares the public interface for the HBasicModeler
**      class.
**
**  HMR FUNCTIONS
**         HBasicModeler_Constructor
**         HBasicModeler_ConstructorFromMatrix
**         HBasicModeler_ConstructorFromTrfModel
**         HBasicModeler_CopyConstructor
**         HBasicModeler_Copy
**         HBasicModeler_Destructor
**         HBasicModeler_AddRotation
**         HBasicModeler_AddScaling
**         HBasicModeler_AddTranslation
**         HBasicModeler_AddMirroring
**         HBasicModeler_AddAffinity
**         HBasicModeler_AddModeler
**         HBasicModeler_AddTrfModel
**         HBasicModeler_AddSlo
**         HBasicModeler_RemoveSlo
**         HBasicModeler_SetRotation
**         HBasicModeler_SetTranslation
**         HBasicModeler_SetScaling
**         HBasicModeler_SetAffinity
**         HBasicModeler_SetMatrix
**         HBasicModeler_GetTranslation
**         HBasicModeler_GetScaling
**         HBasicModeler_GetRotation
**         HBasicModeler_GetAffinity
**         HBasicModeler_GetMatrix
**         HBasicModeler_InvertModel
**         HBasicModeler_ApplyToPoint
**         HBasicModeler_BuildModel
**         HBasicModeler_PrintMatrix
**  END HMR FUNCTIONS
**
**====================================================================*/
#ifndef __HBASEMOD_H__
#define __HBASEMOD_H__

#include "oldhtypes.h"
#include "trfmodel.h"


/*
** Implementation header
*/
#if !defined(__HBASEMOD_HI__) && defined(CLASS_IMPLEMENTATION)
#include "hbasemod.hi"
#endif

HDECLARE_CLASS (HBasicModeler, HBasicModelerAttr);


/*
** Local error messages
*/

/*
** Usefull defines
*/
#define MIRROR_VERTICAL     0
#define MIRROR_HORIZONTAL   1
#define MIRROR_UPDOWN       2
#define MIRROR_DOWNUP       3

#define MATRIX_NB_ROW       4
#define MATRIX_NB_COL       4

/*
** SLO
*/
#define HBM_ULV 0
#define HBM_URV 1
#define HBM_LLV 2
#define HBM_LRV 3
#define HBM_ULH 4
#define HBM_URH 5
#define HBM_LLH 6
#define HBM_LRH 7

/*====================================================================
** Public methods
**====================================================================*/
#if defined (__cplusplus)
extern "C" {
#endif

HSTATUS HBasicModeler_Constructor             (HBasicModeler  *p_pHBasicModeler);

HSTATUS HBasicModeler_ConstructorFromMatrix   (HBasicModeler  *p_pHBasicModeler,
                                                          pMAT_           pi_pMatrix);

HSTATUS HBasicModeler_ConstructorFromTrfModel (HBasicModeler  *p_pHBasicModeler,
                                                          TRFHANDLE       pi_modelHandle);

HSTATUS HBasicModeler_CopyConstructor    (HBasicModeler       *p_pHBasicModeler,
                                                     const HBasicModeler *pi_pHBasicModeler );

HSTATUS HBasicModeler_Copy               (      HBasicModeler  *p_pHBasicModeler,
                                                     const HBasicModeler  *pi_pHBasicModeler );

HSTATUS HBasicModeler_Destructor         (HBasicModeler       *p_pHBasicModeler);

HSTATUS HBasicModeler_AddRotation        (HBasicModeler       *p_pHBasicModeler,
                                                     const DCOORD        *pi_pRotationPoint,
                                                     double              pi_Rotation);

HSTATUS HBasicModeler_AddScaling         (HBasicModeler       *p_pHBasicModeler,
                                                     const DCOORD        *pi_pScalePoint,
                                                     double              pi_Scaling);

HSTATUS HBasicModeler_AddScalingXY       (HBasicModeler       *p_pHBasicModeler,
                                                     const DCOORD        *pi_pScalePoint,
                                                     double              pi_ScalingX,
                                                     double              pi_ScalingY);

HSTATUS HBasicModeler_AddTranslation     (HBasicModeler       *p_pHBasicModeler,
                                                     double              pi_XOffset,
                                                     double              pi_YOffset);

HSTATUS HBasicModeler_AddMirroring       (HBasicModeler       *p_pHBasicModeler,
                                                     const DCOORD        *pi_pMirrorPoint,
                                                     int8_t              pi_Horizontal,
                                                     int8_t              pi_Vertical,
                                                     int8_t              pi_UpDown,
                                                     int8_t              pi_DownUp);

HSTATUS HBasicModeler_AddAffinity        (HBasicModeler       *p_pHBasicModeler,
                                                     double              pi_Affinity);

HSTATUS HBasicModeler_AddAffinityAboutPt (HBasicModeler       *p_pHBasicModeler,
                                                     double              pi_affAngle,
                                                     DCOORD              *pi_pPtUU );

HSTATUS HBasicModeler_SetScaling         (HBasicModeler       *p_pHBasicModeler,
                                                     double              pi_XScale,
                                                     double              pi_YScale);

HSTATUS HBasicModeler_SetTranslation     (HBasicModeler       *p_pHBasicModeler,
                                                     double              pi_XOffset,
                                                     double              pi_YOffset);

HSTATUS HBasicModeler_SetRotation        (HBasicModeler       *p_pHBasicModeler,
                                                     double              pi_pRotation);

HSTATUS HBasicModeler_SetAffinity               (HBasicModeler       *p_pHBasicModeler,
                                                                                                         double              pi_Affinity);

HSTATUS HBasicModeler_SetMatrix          (HBasicModeler       *p_pHBasicModeler,
                                                     pMAT_                pi_pMatrix);

HSTATUS HBasicModeler_BuildModel         (const HBasicModeler *p_pHBasicModeler,
                                                     TRFHANDLE            pio_TransfoModel);


HSTATUS HBasicModeler_GetMatrix          (HBasicModeler       *p_pHBasicModeler,
                                                     pMAT_                po_pMatrix);

HSTATUS HBasicModeler_GetTranslation     (HBasicModeler       *p_pHBasicModeler,
                                                     double             *po_pXOffset,
                                                     double             *po_pYOffset);

HSTATUS HBasicModeler_GetScaling         (HBasicModeler       *p_pHBasicModeler,
                                                     double             *po_pXScale,
                                                     double             *po_pYScale);

HSTATUS HBasicModeler_GetRotation        (HBasicModeler       *p_pHBasicModeler,
                                                     double             *po_pRotation);

HSTATUS HBasicModeler_GetAffinity               (HBasicModeler           *p_pHBasicModeler,
                                                                                                         double                         *po_pAffinity);

HSTATUS HBasicModeler_AddTrfModel        (HBasicModeler       *p_pHBasicModeler,
                                                     TRFHANDLE            trfHandle);

HSTATUS HBasicModeler_AddSlo             (HBasicModeler       *p_pHBasicModeler,
                                                     int32_t              pi_Slo);

HSTATUS HBasicModeler_RemoveSlo          (HBasicModeler       *p_pHBasicModeler,
                                                     int32_t              pi_pSlo);

HSTATUS HBasicModeler_GetSlo             (HBasicModeler       *p_pHBasicModeler,
                                                     HBasicModeler       *po_pModelWithoutSlo,
                                                     int32_t             *po_pSlo);

HSTATUS HBasicModeler_AddModeler         (HBasicModeler       *p_pHBasicModeler,
                                                     HBasicModeler       *pi_pModeler);

HSTATUS HBasicModeler_InvertModel        (HBasicModeler       *p_pHBasicModeler);

HSTATUS HBasicModeler_ApplyToPoint       (      HBasicModeler       *p_pHBasicModeler,
                                                     const DCOORD              *pi_pInputPt,
                                                           DCOORD              *po_pOutputPt);

HSTATUS HBasicModeler_PrintMatrix        (HBasicModeler       *p_pHBasicModeler);


#if defined (__cplusplus)
}
#endif

#endif /* __HBASEMOD_H__ */
