/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/hbasemod.c $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================== HMR
**
**  hbasemod.c
**
**      This file implements the HBasicModeler class.
**
**========================================================================*/

#define OBJECT p_pHBasicModeler
#define PRIVATE_CLASS

#include <string.h>

#include "hbasemod.h"
#include "assert.h"
#include "stdio.h"

/*===================================================================
**  Local constant definitions
**==================================================================*/
#define HBM_RADANGLETOLERANCE 0.00000000000001  /* Tolerance to evaluate the value */
                                                /* of an angle in radians */

/*===================================================================
** Local Macro definitions
**==================================================================*/
/*
** Test if a value in the matrix is zero
*/
#define HBM_IS_ZERO(a)             (fabs(a) < HBM_RADANGLETOLERANCE)

/*
** Test if an angle is zero
*/
#define HBM_IS_ZERO_RADIANS(a)     (fabs(a) < HBM_RADANGLETOLERANCE)

/*
** Test if a value in the matrix is not zero
*/
#define HBM_ISNOT_ZERO(a)          (fabs(a) > HBM_RADANGLETOLERANCE)

/*
** Test if an angle is not zero
*/
#define HBM_ISNOT_ZERO_RADIANS(a)  (fabs(a) > HBM_RADANGLETOLERANCE)

/*
** Test if a value equal to one
*/
#define HBM_IS_ONE(a)              (fabs((1.0) - (a)) < HBM_RADANGLETOLERANCE)

/*
** A more precise evaluaton of PI
*/
#define HBM_PI                     (3.0 * (acos(0.5)))


/*==================================================================
** Static prototypes
**==================================================================*/
static HSTATUS sAddTranslation                 (pMAT_           *matrix,
                                                double          pi_XOffset,
                                                double          pi_YOffset);

static HSTATUS sAddScale                       (pMAT_           *matrix,
                                                const DCOORD    *pi_pScalePoint,
                                                double          pi_ScaleFactorX,
                                                double          pi_ScaleFactorY);

static HSTATUS sAddMirror                      (pMAT_           *matrix,
                                                const DCOORD    *pi_MirrorPoint,
                                                int32_t          pi_MirrorType);

static HSTATUS sAddRotation                    (pMAT_           *matrix,
                                                const DCOORD    *pi_pRotationPoint,
                                                double          pi_Rotation);

static HSTATUS sAddAffinity                    (pMAT_           *matrix,
                                                const DCOORD    *pi_pAffinityPoint,
                                                double          pi_Affinity);

static HSTATUS sAddTransformation              (pMAT_           *A,
                                                pMAT_           *B);

static HSTATUS sSetBasicMirrorMatrix           (pMAT_            matrix,
                                                int32_t          mirrorType);

static HSTATUS sSetBasicScalingMatrix          (pMAT_            ScalingMatrix,
                                                double          pi_ScaleFactorX,
                                                double          pi_ScaleFactorY);

static HSTATUS sSetBasicRotationMatrix         (pMAT_            RMatrix,
                                                double          pi_Rotation);

static HSTATUS sSetBasicTranslationMatrix      (pMAT_            TransformMatrix,
                                                double          pi_XOffset,
                                                double          pi_YOffset);

static HSTATUS sSetBasicAffinityMatrix         (pMAT_            SHMatrix,
                                                double          pi_Affinity);

static HSTATUS sInvertMatrix                   (pMAT_            pi_pInputMatrix,
                                                pMAT_            po_pOutputMatrix);

static HSTATUS sAdjoint                        (pMAT_            in,
                                                pMAT_            out);


double det4x4      (pMAT_ mat);
double det3x3      (double a1, double a2, double a3,
                     double b1, double b2, double b3,
                     double c1, double c2, double c3);

/*==========================================================================
** Public Methods
**==========================================================================*/
/***************************************************************************
**  HBasicModeler_Constructor
**
**
**  Stephane Poulin          Modified 5/11/1997
****************************************************************************/
HSTATUS HBasicModeler_Constructor      (HBasicModeler    *p_pHBasicModeler)
{
    int8_t modelerConstructed = FALSE;
    int8_t matrixAllocated    = FALSE;

    HDEF (Status, H_SUCCESS);

    HPRECONDITION (p_pHBasicModeler);

    /*
    ** Allocate memory for the object
    */
    if (NULL == (THIS = (HBasicModeler)malloc (sizeof (HBasicModelerAttr))))
    {
        HSET (Status, H_NOTENOUGHMEMORY);
        goto WRAPUP;
    }

    modelerConstructed = TRUE;

    /*
    ** Allocate memory for the transformation matrix
    */
    THIS->m_pMatrix = mat_create(MATRIX_NB_ROW, MATRIX_NB_COL);
    if (THIS->m_pMatrix == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    matrixAllocated = TRUE;

    /*
    ** Set matrix to Identity
    */
    if(HISERROR(HSET(Status, mat_setIdentity (THIS->m_pMatrix))))
        goto WRAPUP;


WRAPUP:
    if(HISERROR(Status) && modelerConstructed && matrixAllocated)
        HBasicModeler_Destructor(p_pHBasicModeler);
    else if(HISERROR(Status) && modelerConstructed)
        free(THIS);

    HRET (Status);
}

/***************************************************************************
**  HBasicModeler_ConstructorFromMatrix
**
**  DESCRIPTION
**
**    This function create a HBasicModeler instance and initialise the
**    transformation Matrix of this object with the matrix recieved in
**    parameter.
**
**
**  PARAMETERS
**
**    p_pHBasicModeler   -   Pointer to a HBasicModeler object
**    pi_pMatrix         -   Pointer to the a matrix
**
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
****************************************************************************/
HSTATUS HBasicModeler_ConstructorFromMatrix (HBasicModeler *p_pHBasicModeler,
                                             pMAT_          pi_pMatrix)
{
    int8_t modelerConstructed = FALSE;

    HDEF (Status, H_SUCCESS);

    HPRECONDITION (pi_pMatrix);


    if (HISERROR(HSET(Status, HBasicModeler_Constructor (p_pHBasicModeler))))
        goto WRAPUP;

    modelerConstructed = TRUE;

    /*
    ** Copy transformation matrix
    */
    if(HISERROR(HSET(Status, mat_copyData (THIS->m_pMatrix, pi_pMatrix))))
        goto WRAPUP;


WRAPUP:
    if(HISERROR(Status) && modelerConstructed)
        HBasicModeler_Destructor(p_pHBasicModeler);

    HRET (Status);
}

/***************************************************************************
**  HBasicModeler_ConstructorFromTrfModel
**
**  DESCRIPTION
**
**    This function create a HBasicModeler instance and initialise the
**    transformation Matrix of this object with the TrfModel recieved in
**    parameter.
**
**
**  PARAMETERS
**
**    p_pHBasicModeler   -   Pointer to a HBasicModeler object
**    pi_modelHandle     -   Handle of the TrfModel
**
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
****************************************************************************/
HSTATUS HBasicModeler_ConstructorFromTrfModel (HBasicModeler *p_pHBasicModeler,
                                               TRFHANDLE      pi_modelHandle)
{
    COEFFICIENTS_3D coefs;
    int32_t         modelType;
    int8_t         modelerConstructed = FALSE;

    HDEF (Status, H_SUCCESS);


    if (HISERROR(HSET(Status, HBasicModeler_Constructor (p_pHBasicModeler))))
        goto WRAPUP;

    modelerConstructed = TRUE;

    /*
    ** Convert TRFModel to Matrix
    */
    if (HISERROR(HSET(Status, trf_modelGetCoefficients (pi_modelHandle, &coefs))))
    {
        goto WRAPUP;
    }

    if (HISERROR(HSET(Status, trf_modelGetType(pi_modelHandle, &modelType))))
    {
        goto WRAPUP;
    }

    switch (modelType)
    {
        case TRANSFO_TRAN_2D:
            mat_v(THIS->m_pMatrix, 0, 3) = (double) coefs.coef[0][0][0];
            mat_v(THIS->m_pMatrix, 1, 3) = (double) coefs.coef[0][1][0];
            break;

        case TRANSFO_HELM_2D:
        case TRANSFO_SIMI_2D:
        case TRANSFO_AFFI_2D:
            mat_v(THIS->m_pMatrix, 0, 0) = (double)  coefs.coef[0][0][1];
            mat_v(THIS->m_pMatrix, 0, 1) = (double)  coefs.coef[0][0][2];
            mat_v(THIS->m_pMatrix, 0, 3) = (double) (  coefs.coef[1][0][0]
                                                     + coefs.coef[0][0][3]
                                                     - (coefs.coef[0][0][0]*coefs.coef[0][0][1])
                                                     - (coefs.coef[0][1][0]*coefs.coef[0][0][2]));
            mat_v(THIS->m_pMatrix, 1, 0) = (double) coefs.coef[0][1][1];
            mat_v(THIS->m_pMatrix, 1, 1) = (double) coefs.coef[0][1][2];
            mat_v(THIS->m_pMatrix, 1, 3) = (double) (  coefs.coef[1][1][0]
                                                     + coefs.coef[0][1][3]
                                                     - (coefs.coef[0][0][0]*coefs.coef[0][1][1])
                                                     - (coefs.coef[0][1][0]*coefs.coef[0][1][2]));
            break;

        case TRANSFO_MAT_2D:
            mat_v(THIS->m_pMatrix, 0, 0) = (double) coefs.coef[0][0][1];
            mat_v(THIS->m_pMatrix, 0, 1) = (double) coefs.coef[0][0][2];
            mat_v(THIS->m_pMatrix, 0, 3) = (double) coefs.coef[0][0][0];
            mat_v(THIS->m_pMatrix, 1, 0) = (double) coefs.coef[0][1][1];
            mat_v(THIS->m_pMatrix, 1, 1) = (double) coefs.coef[0][1][2];
            mat_v(THIS->m_pMatrix, 1, 3) = (double) coefs.coef[0][1][0];
            break;

        default:
            HSET(Status, HERROR);
            goto WRAPUP;
    }


WRAPUP:
    if (HISERROR(Status) && modelerConstructed)
        HBasicModeler_Destructor(p_pHBasicModeler);

    HRET (Status);
}

/***************************************************************************
**  HBasicModeler_Destructor
**
**
**  Stephane Poulin         Modified  5/11/1997
****************************************************************************/
HSTATUS HBasicModeler_Destructor       (HBasicModeler  *p_pHBasicModeler)
{
    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;

    /*
    ** Destroy the pointer to the transformation matrix
    */
    mat_destroy(THIS->m_pMatrix);

    /*
    ** Destroy the HBasicModeler object
    */
    free (THIS);

    HRET (Status);
}

/***************************************************************************
**  HBasicModeler_CopyConstructor
**
**  DESCRIPTION
**
**    This function create a copy of HBasicModeler instance.
**
**
**  PARAMETERS
**
**    p_pHBasicModeler   -   Pointer to a HBasicModeler object
**    pi_pHBasicModeler  -   Pointer to the HBasicModeler to copy
**
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
****************************************************************************/
HSTATUS HBasicModeler_CopyConstructor      (      HBasicModeler  *p_pHBasicModeler,
                                            const HBasicModeler  *pi_pHBasicModeler )
{
    int8_t copyConstructed = FALSE;

    HDEF (Status, H_SUCCESS);

    HPRECONDITION (pi_pHBasicModeler);


    if (HISERROR(HSET(Status, HBasicModeler_Constructor (p_pHBasicModeler))))
        goto WRAPUP;

    copyConstructed = TRUE;

    if (HISERROR(HSET(Status, HBasicModeler_Copy(&THIS, pi_pHBasicModeler))))
        goto WRAPUP;


WRAPUP:
    if(HISERROR(Status) && copyConstructed)
        HBasicModeler_Destructor(p_pHBasicModeler);

    HRET (Status);
}

/***************************************************************************
**  HBasicModeler_Copy
**
**  DESCRIPTION
**
**    This function copy the source Modeler to the destination modeler
**    The destination modeler must be allocated before calling this method
**
**  PARAMETERS
**
**    p_pHBasicModeler   -   Destination Modeler (Already Constructed)
**    pi_pHBasicModeler  -   Source Modeler
**
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin   FEB 1999  Original version
****************************************************************************/
HSTATUS HBasicModeler_Copy  (      HBasicModeler  *p_pHBasicModeler,
                             const HBasicModeler  *pi_pHBasicModeler )
{
    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;
    HPRECONDITION (pi_pHBasicModeler);

    /*
    ** Copy transformation matrix
    */
    HISERROR(HSET(Status, mat_copyData ((*p_pHBasicModeler)->m_pMatrix,
                                           (*pi_pHBasicModeler)->m_pMatrix)));


    HRET (Status);
}
/***************************************************************************
**  HBasicModeler_AddRotation
**
**  DESCRIPTION
**
**    This function add rotation (rad) to the transformation Matrix of the
**    input HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler   <-> Pointer to a HBasicModeler object
**    pi_pRotationPoint  ->  Point to rotate about
**    pi_Rotation        ->  Rotation angle in radians
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin
****************************************************************************/
HSTATUS HBasicModeler_AddRotation        (HBasicModeler       *p_pHBasicModeler,
                                          const DCOORD        *pi_pRotationPoint,
                                          double              pi_Rotation)
{
    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;
    HPRECONDITION (pi_pRotationPoint);


    if(HISERROR(HSET(Status, sAddRotation(&(THIS->m_pMatrix), pi_pRotationPoint, pi_Rotation))))
        goto WRAPUP;

WRAPUP:
    HRET (Status);
}

/***************************************************************************
**  HBasicModeler_AddAffinity
**
**  DESCRIPTION
**
**    This method add affinity (rad) to the transformation Matrix of the
**    input HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler   <-> Pointer to a HBasicModeler object
**    pi_Affinity        ->  Affinity angle in radians
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin
****************************************************************************/
HSTATUS HBasicModeler_AddAffinity        (HBasicModeler       *p_pHBasicModeler,
                                          double              pi_Affinity)
{
        double ActualAffinity_rad;
    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;

        if(HISERROR(HSET(Status, HBasicModeler_GetAffinity(&THIS, &ActualAffinity_rad))))
                goto WRAPUP;

        if(HISERROR(HSET(Status, HBasicModeler_SetAffinity(&THIS, ActualAffinity_rad + pi_Affinity))))
                goto WRAPUP;

WRAPUP:
    HRET (Status);
}

/*sh=========================================================================
**  HBasicModeler_AddAffinityAboutPt
**
**  DESCRIPTION
**    This method add affinity (rad) to the transformation Matrix of the
**    input HBasicModeler object, relative to a certain point.
**
**  PARAMETERS
**      p_pHBasicModeler - a constructed modeler
**      pi_affAngle      - affinity angle (rad)
**      pi_pPtUU         - pt. about which affinity is applied (this
**                         point will remain at the same position)
**
**  RETURN VALUE
**
**
**  Eric Paquet    May 98 - Original version
**===========================================================================*/
HSTATUS HBasicModeler_AddAffinityAboutPt
                   (       HBasicModeler *p_pHBasicModeler,
                           double        pi_affAngle,
                           DCOORD        *pi_pPtUU )
{
    double         tr_x, tr_y;
    TRFHANDLE       trfModel;
    int8_t         trfModelOk = FALSE;
    DCOORD          ptNoTrfUU;
    DCOORD          ptNew;
    HDEF            (status, H_SUCCESS);

    /*
    ** Construct the trfModel
    */
    if(-1 == (trfModel = trf_modelNew()))
    {
        HSET(status, HERROR);
        goto WRAPUP;
    }
    trfModelOk = TRUE;

    /*
    ** Create the model from the modeler
    */
    if(HISERROR(HSET(status, HBasicModeler_BuildModel (&THIS, trfModel))))
        goto WRAPUP;

    /*
    ** Transform point to its position when no transformation
    ** is applied.
    */
    trf_modelTransfoCoord (  trfModel,
                             pi_pPtUU,
                            &ptNoTrfUU,
                             TRANSFO_INVERSE );

    /*
    ** Set the rotation angle
    */
    if(HISERROR(HSET(status, HBasicModeler_AddAffinity (&THIS, pi_affAngle))))
        goto WRAPUP;

    /*
    ** Create the new model from the modeler
    */
    if(HISERROR(HSET(status, HBasicModeler_BuildModel (&THIS, trfModel))))
        goto WRAPUP;

    /*
    ** Transform input point to its new position .
    */
    trf_modelTransfoCoord (  trfModel,
                            &ptNoTrfUU,
                            &ptNew,
                             TRANSFO_DIRECT );

    /*
    ** Get translation created by the affinity
    */
    tr_x = pi_pPtUU->x - ptNew.x;
    tr_y = pi_pPtUU->y - ptNew.y;

    /*
    ** add a translation to the modeler
    */
    if(HISERROR(HSET(status, HBasicModeler_AddTranslation (&THIS, tr_x, tr_y))))
        goto WRAPUP;

WRAPUP:

    if (trfModelOk)    trf_modelDestroy (trfModel);

    HRET (status);

}

/***************************************************************************
**  HBasicModeler_AddScaling
**
**  DESCRIPTION
**
**    This method add a scale to the transformation Matrix of the
**    input HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler   <-> Pointer to a HBasicModeler object
**    pi_pScalePoint     ->  Point to Scale about
**    pi_Scaling         ->  Scaling factor
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin
****************************************************************************/
HSTATUS HBasicModeler_AddScaling         (HBasicModeler       *p_pHBasicModeler,
                                          const DCOORD        *pi_pScalePoint,
                                          double              pi_Scaling)
{
    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;
    HPRECONDITION (pi_pScalePoint);


    /*
    ** Scale Modeler Matrix
    */
    if (HISERROR(HSET(Status, sAddScale (&(THIS->m_pMatrix),
                                              pi_pScalePoint,
                                              pi_Scaling,
                                              pi_Scaling))))
    {
        goto WRAPUP;
    }


WRAPUP:
    HRET (Status);
}


/***************************************************************************
**  HBasicModeler_AddScalingXY
**
**  DESCRIPTION
**
**    This method add a scale (specific in X and Y) to the transformation
**    Matrix of the input HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler   <-> Pointer to a HBasicModeler object
**    pi_pScalePoint     ->  Point to Scale about
**    pi_Scaling[X|Y]    ->  Scaling factor
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Eric Paquet  April 1998
****************************************************************************/
HSTATUS HBasicModeler_AddScalingXY       (HBasicModeler       *p_pHBasicModeler,
                                          const DCOORD        *pi_pScalePoint,
                                          double              pi_ScalingX,
                                          double              pi_ScalingY)
{
    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;
    HPRECONDITION (pi_pScalePoint);


    /*
    ** Scale Modeler Matrix
    */
    if (HISERROR(HSET(Status, sAddScale (&(THIS->m_pMatrix),
                                              pi_pScalePoint,
                                              pi_ScalingX,
                                              pi_ScalingY))))
    {
        goto WRAPUP;
    }


WRAPUP:
    HRET (Status);
}


/***************************************************************************
**  HBasicModeler_SetScaling
**
**  DESCRIPTION
**
**    This method set a scale in the transformation Matrix of the
**    input HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler   <-> Pointer to a HBasicModeler object
**    pi_XScale          ->  X Scaling factor
**    pi_YScale          ->  Y Scaling factor
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin
****************************************************************************/
HSTATUS HBasicModeler_SetScaling         (HBasicModeler       *p_pHBasicModeler,
                                          double              pi_XScale,
                                          double              pi_YScale)
{
    pMAT_       Matrix;
    double     ScaleX, ScaleY;

    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;


    /*
    ** Cannot set negative value or zero
    */
    if (pi_XScale <= 0.0 || pi_YScale <= 0.0)
    {
        HSET (Status, HERROR);
        goto WRAPUP;
    }

    if (HISERROR(HSET(Status, HBasicModeler_GetScaling(p_pHBasicModeler, &ScaleX, &ScaleY))))
        goto WRAPUP;

    Matrix = THIS->m_pMatrix;

    /*
    ** Set new scale to the current modeler Matrix
    */
    mat_v(Matrix, 0, 0) = mat_v(Matrix, 0, 0) * pi_XScale / ScaleX;
    mat_v(Matrix, 1, 0) = mat_v(Matrix, 1, 0) * pi_XScale / ScaleX;
    mat_v(Matrix, 0, 1) = mat_v(Matrix, 0, 1) * pi_YScale / ScaleY;
    mat_v(Matrix, 1, 1) = mat_v(Matrix, 1, 1) * pi_YScale / ScaleY;


WRAPUP:
    HRET (Status);
}


/***************************************************************************
**  HBasicModeler_AddTranslation
**
**  DESCRIPTION
**
**    This method add a translation to the transformation Matrix of the
**    input HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler   <-> Pointer to a HBasicModeler object
**    pi_XOffset          ->  X translation
**    pi_YOffset          ->  Y translation
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin
****************************************************************************/
HSTATUS HBasicModeler_AddTranslation     (HBasicModeler       *p_pHBasicModeler,
                                          double              pi_XOffset,
                                          double              pi_YOffset)
{

    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;

    /*
    ** Translate Modeler Matrix
    */
    if (HISERROR(HSET(Status, sAddTranslation (&(THIS->m_pMatrix),
                                                  pi_XOffset,
                                                  pi_YOffset))))
    {
        goto WRAPUP;
    }


WRAPUP:

    HRET (Status);


}

/***************************************************************************
**  HBasicModeler_SetTranslation
**
**  DESCRIPTION
**
**    This method add a translation to the transformation Matrix of the
**    input HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler   <-> Pointer to a HBasicModeler object
**    pi_XOffset          ->  X translation
**    pi_YOffset          ->  Y translation
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin
****************************************************************************/
HSTATUS HBasicModeler_SetTranslation     (HBasicModeler       *p_pHBasicModeler,
                                          double              pi_XOffset,
                                          double              pi_YOffset)
{
    pMAT_ Matrix;

    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;


    /*
    ** Set new translation into the current modeler Matrix
    */
    Matrix = THIS->m_pMatrix;

    mat_v(Matrix, 0, 3) = pi_XOffset;
    mat_v(Matrix, 1, 3) = pi_YOffset;


    HRET (Status);
}

/***************************************************************************
**  HBasicModeler_AddMirroring
**
**  DESCRIPTION
**
**    This method add a mirror to the transformation Matrix of the
**    input HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler   <-> Pointer to a HBasicModeler object
**    pi_pMirrorPoint    ->  Poit to mirror about
**    pi_Horizontal      ->  True if Horizontal mirror
**    pi_Vertical        ->  True if Vertical mirror
**    pi_UpDown          ->  True if Up down mirror
**    pi_DownUp          ->  True if Down up mirror
**
**  NOTE
**    Only one flag can be true.
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin
****************************************************************************/
HSTATUS HBasicModeler_AddMirroring       (HBasicModeler       *p_pHBasicModeler,
                                          const DCOORD        *pi_pMirrorPoint,
                                          int8_t              pi_Horizontal,
                                          int8_t              pi_Vertical,
                                          int8_t              pi_UpDown,
                                          int8_t              pi_DownUp)
{
    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;
    HPRECONDITION (pi_pMirrorPoint);

    /*
    ** Allow only one mirror to be set
    */
    if (((int32_t) pi_Horizontal + (int32_t) pi_Vertical + (int32_t) pi_UpDown +
                                                       (int32_t) pi_DownUp) > 1)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    /*
    ** Apply mirror to the transformation matrix
    */
    if (pi_Horizontal)
        Status = sAddMirror(&(THIS->m_pMatrix), pi_pMirrorPoint, MIRROR_HORIZONTAL);
    else if (pi_Vertical)
        Status = sAddMirror(&(THIS->m_pMatrix), pi_pMirrorPoint, MIRROR_VERTICAL);
    else if (pi_UpDown)
        Status = sAddMirror(&(THIS->m_pMatrix), pi_pMirrorPoint, MIRROR_UPDOWN);
    else if (pi_DownUp)
        Status = sAddMirror(&(THIS->m_pMatrix), pi_pMirrorPoint, MIRROR_DOWNUP);

WRAPUP:
    HRET (Status);
}

/***************************************************************************
**  HBasicModeler_AddSlo
**
**  DESCRIPTION
**
**    This method adds a SLO transformation to the transformation matrix of the
**    input HBasicModeler object.
**    The transformation allow to switch from SLO 4 (ULH) to any other SLO.
**
**  PARAMETERS
**
**    p_pHBasicModeler   <-> Pointer to a HBasicModeler object
**    pi_Slo             ->  SLO code. (See HBasemod.h)
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin        FEB 1998
**===========================================================================*/
HSTATUS HBasicModeler_AddSlo   (HBasicModeler  *p_pHBasicModeler,
                                int32_t         pi_Slo)
{
    DCOORD_3D       Origin;

    HDEF(Status, HSUCCESS);
    HOBJECT_PRECONDITIONS;

    HBasicModeler_GetTranslation(&THIS, &Origin.x, &Origin.y);

    /*
    ** Add SLO to the transformation matrix
    */
    switch (pi_Slo)
    {
    case HBM_ULV: /* 0 ULV */
        {
            HBasicModeler_AddMirroring(&THIS, &Origin, FALSE, TRUE, FALSE, FALSE);
            HBasicModeler_AddRotation (&THIS, &Origin, PI/2);
        }
        break;
    case HBM_URV: /* 1 URV */
        {
            HBasicModeler_AddRotation (&THIS, &Origin, -(PI/2));
        }
        break;
    case HBM_LLV: /* 2 LLV */
        {
            HBasicModeler_AddRotation   (&THIS, &Origin, PI/2);
        }
        break;
    case HBM_LRV: /* 3 LRV */
        {
            HBasicModeler_AddRotation (&THIS, &Origin, -(PI/2));
            HBasicModeler_AddMirroring(&THIS, &Origin, TRUE, FALSE, FALSE, FALSE);
        }
        break;
    case HBM_ULH: /* 4 ULH */
        {

        }
        break;
    case HBM_URH: /* 5 URH */
        {
            HBasicModeler_AddMirroring  (&THIS, &Origin, FALSE, TRUE, FALSE, FALSE);
        }
        break;
    case HBM_LLH: /* 6 LLH */
        {
            HBasicModeler_AddMirroring(&THIS, &Origin, TRUE, FALSE, FALSE, FALSE);
        }

        break;
    case HBM_LRH: /* 7 LRH */
        {
            HBasicModeler_AddMirroring(&THIS, &Origin, TRUE, FALSE, FALSE, FALSE);
            HBasicModeler_AddMirroring(&THIS, &Origin, FALSE, TRUE, FALSE, FALSE);
        }
        break;
    default:
        {
            HSET(Status, HERROR);
            goto WRAPUP;
        }
    }

WRAPUP:

    HRET(Status);
}

/***************************************************************************
**  HBasicModeler_RemoveSlo
**
**  DESCRIPTION
**
**    This method remove a SLO from the transformation matrix of the
**    input HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler   <-> Pointer to a HBasicModeler object
**    pi_Slo             ->  SLO code. (See HBasemod.h)
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin
****************************************************************************/
HSTATUS HBasicModeler_RemoveSlo ( HBasicModeler  *p_pHBasicModeler,
                                  int32_t         pi_pSlo)
{
    DCOORD        Origin;
    HDEF(Status, HSUCCESS);
    HOBJECT_PRECONDITIONS;

    if(HISERROR(HSET(Status, HBasicModeler_GetTranslation(&THIS, &Origin.x, &Origin.y))))
        goto WRAPUP;

    /*
    ** Remove SLO from transformation matrix
    */
    switch (pi_pSlo)
    {
    case HBM_ULV: /* 0 ULV */
        {
            HBasicModeler_AddRotation (&THIS, &Origin, -PI/2);
            HBasicModeler_AddMirroring(&THIS, &Origin, FALSE, TRUE, FALSE, FALSE);
        }
        break;
    case HBM_URV: /* 1 URV */
        {
            HBasicModeler_AddRotation (&THIS, &Origin, (PI/2));
        }
        break;
    case HBM_LLV: /* 2 LLV */
        {
            HBasicModeler_AddRotation   (&THIS, &Origin, -PI/2);
        }
        break;
    case HBM_LRV: /* 3 LRV */
        {
            HBasicModeler_AddMirroring(&THIS, &Origin, TRUE, FALSE, FALSE, FALSE);
            HBasicModeler_AddRotation (&THIS, &Origin, (PI/2));
        }
        break;
    case HBM_ULH: /* 4 ULH */
        {

        }
        break;
    case HBM_URH: /* 5 URH */
        {
            HBasicModeler_AddMirroring  (&THIS, &Origin, FALSE, TRUE, FALSE, FALSE);
        }
        break;
    case HBM_LLH: /* 6 LLH */
        {
            HBasicModeler_AddMirroring(&THIS, &Origin, TRUE, FALSE, FALSE, FALSE);
        }

        break;
    case HBM_LRH: /* 7 LRH */
        {
            HBasicModeler_AddMirroring(&THIS, &Origin, FALSE, TRUE, FALSE, FALSE);
            HBasicModeler_AddMirroring(&THIS, &Origin, TRUE, FALSE, FALSE, FALSE);
        }
        break;
    default:
        HSET(Status, HERROR);
        goto WRAPUP;
    }

WRAPUP:
    HRET(Status);
}
/***************************************************************************
**  HBasemodeler_GetSlo
**
** This method finds the equivalent SLO to the rotation and mirrors presents in
** the HBasicModeler Transformation matrix.
**
** PARAMETERS
**   p_pHBasicModeler     -  Input Basic Modeler
**   po_pModelWithoutSlo  -  Output Basic Modeler. This modeler represent the
**                           transformation matrix from witch all mirrors and
**                           orthogonal rotations has been removed.
**                           This Parameter can be NULL.
**  po_pSlo               -  SLO equivalent to the orthogonals rotations and
**                           mirrors removed from input modeler object.
**
**  Stephane Poulin          MARCH 1998
****************************************************************************/
HSTATUS HBasicModeler_GetSlo (HBasicModeler  *p_pHBasicModeler,
                              HBasicModeler  *po_pModelWithoutSlo,
                              int32_t        *po_pSlo)
{
    HBasicModeler Modeler_Slo4;
    double       Affinity;
    double       Rotation;
    DCOORD        Origin;
    double       ScaleX;
    double       ScaleY;
    double       A00, A01, A10, A11;
    pMAT_         matrix;

    int8_t       isModelerAlloc = FALSE;
    int8_t       isMatrixAlloc  = FALSE;

    HDEF(Status, HSUCCESS);
    HOBJECT_PRECONDITIONS;
    HPRECONDITION(po_pSlo);


    /*
    ** Create a modeler.
    */
    if(HISERROR(HSET(Status, HBasicModeler_Constructor(&Modeler_Slo4))))
        goto WRAPUP;
    isModelerAlloc = TRUE;

    /*
    ** Get rotation and affinity of input modeler
    */
    if(HISERROR(HSET(Status, HBasicModeler_GetAffinity(&THIS, &Affinity))))
        goto WRAPUP;
    if(HISERROR(HSET(Status, HBasicModeler_GetRotation(&THIS, &Rotation))))
        goto WRAPUP;

    /*
    ** If Pi/2 < Affinity < 3PI/2, Rotation angle is affected by a Horizontal mirror.
    */
    if(cos(Affinity) < 0.0)
    {
        Rotation = 2 * PI - Rotation;
        Affinity = PI - Affinity;
        if(HISERROR(HSET(Status, HBasicModeler_SetAffinity  (&Modeler_Slo4,  Affinity))))
            goto WRAPUP;
    }
    else
    {
        if(HISERROR(HSET(Status, HBasicModeler_SetAffinity(&Modeler_Slo4,  Affinity))))
            goto WRAPUP;
    }

    /*
    ** Round to the closest orthogonal angle
    */
    if(Rotation > PI/4)
    {
        int32_t n;
        n = (int32_t) round(Rotation / (PI/2));
        Rotation = Rotation - n * (PI/2);
    }
    Origin.x = 0.0;
    Origin.y = 0.0;
    Origin.z = 0.0;
    if(HISERROR(HSET(Status, HBasicModeler_AddRotation(&Modeler_Slo4, &Origin, Rotation))))
        goto WRAPUP;

    if(HISERROR(HSET(Status, HBasicModeler_GetScaling(&THIS, &ScaleX, &ScaleY))))
        goto WRAPUP;

    if(HISERROR(HSET(Status, HBasicModeler_SetScaling(&Modeler_Slo4, ScaleX, ScaleY))))
        goto WRAPUP;

    if(HISERROR(HSET(Status, HBasicModeler_GetRotation(&Modeler_Slo4, &Rotation))))
        goto WRAPUP;

    if(HISERROR(HSET(Status, HBasicModeler_GetAffinity(&Modeler_Slo4, &Affinity))))
        goto WRAPUP;

    /*
    ** Copy the matrix without the SLO into the output Modeler if not NULL
    */
    if (po_pModelWithoutSlo != NULL)
    {
        if (MAT_NULL == (matrix = mat_create(MATRIX_NB_ROW, MATRIX_NB_COL)))
        {
            HSET(Status, HERROR);
            goto WRAPUP;
        }
        isMatrixAlloc = TRUE;

        if(HISERROR(HSET(Status, HBasicModeler_GetMatrix(&Modeler_Slo4, matrix))))
            goto WRAPUP;

        if(HISERROR(HSET(Status, HBasicModeler_SetMatrix(po_pModelWithoutSlo, matrix))))
            goto WRAPUP;

        if(HISERROR(HSET(Status, HBasicModeler_GetTranslation(&THIS, &Origin.x, &Origin.y))))
            goto WRAPUP;

        if(HISERROR(HSET(Status, HBasicModeler_SetTranslation(po_pModelWithoutSlo, Origin.x, Origin.y))))
            goto WRAPUP;
    }

    /*
    ** Find a matrix that only contain mirrors and orthogonales rotation
    */
    if(HISERROR(HSET(Status, HBasicModeler_InvertModel(&Modeler_Slo4))))
        goto WRAPUP;

    if(HISERROR(HSET(Status, HBasicModeler_AddModeler(&Modeler_Slo4, &THIS))))
        goto WRAPUP;

    /*
    ** Identify the SLO Matrix
    */
    A00 = mat_v(Modeler_Slo4->m_pMatrix, 0, 0);
    A01 = mat_v(Modeler_Slo4->m_pMatrix, 0, 1);
    A10 = mat_v(Modeler_Slo4->m_pMatrix, 1, 0);
    A11 = mat_v(Modeler_Slo4->m_pMatrix, 1, 1);

    if( HDOUBLE_EQUAL_EPSILON(A00,  0.0) &&
        HDOUBLE_EQUAL_EPSILON(A01, -1.0) &&
        HDOUBLE_EQUAL_EPSILON(A10, -1.0) &&
        HDOUBLE_EQUAL_EPSILON(A11,  0.0)   )
    {
        *po_pSlo = HBM_ULV;
    }
    else if (   HDOUBLE_EQUAL_EPSILON(A00,  0.0) &&
                HDOUBLE_EQUAL_EPSILON(A01,  1.0) &&
                HDOUBLE_EQUAL_EPSILON(A10, -1.0) &&
                HDOUBLE_EQUAL_EPSILON(A11,  0.0)   )
    {
        *po_pSlo = HBM_URV;
    }
    else if (   HDOUBLE_EQUAL_EPSILON(A00,  0.0) &&
                HDOUBLE_EQUAL_EPSILON(A01, -1.0) &&
                HDOUBLE_EQUAL_EPSILON(A10,  1.0) &&
                HDOUBLE_EQUAL_EPSILON(A11,  0.0)   )
    {
        *po_pSlo = HBM_LLV;
    }
    else if (   HDOUBLE_EQUAL_EPSILON(A00,  0.0) &&
                HDOUBLE_EQUAL_EPSILON(A01,  1.0) &&
                HDOUBLE_EQUAL_EPSILON(A10,  1.0) &&
                HDOUBLE_EQUAL_EPSILON(A11,  0.0)   )
    {
        *po_pSlo = HBM_LRV;
    }
    else if (   HDOUBLE_EQUAL_EPSILON(A00,  1.0) &&
                HDOUBLE_EQUAL_EPSILON(A01,  0.0) &&
                HDOUBLE_EQUAL_EPSILON(A10,  0.0) &&
                HDOUBLE_EQUAL_EPSILON(A11,  1.0)   )
    {
        *po_pSlo = HBM_ULH;
    }
    else if (   HDOUBLE_EQUAL_EPSILON(A00, -1.0) &&
                HDOUBLE_EQUAL_EPSILON(A01,  0.0) &&
                HDOUBLE_EQUAL_EPSILON(A10,  0.0) &&
                HDOUBLE_EQUAL_EPSILON(A11,  1.0)   )
    {
        *po_pSlo = HBM_URH;
    }
    else if (   HDOUBLE_EQUAL_EPSILON(A00,  1.0) &&
                HDOUBLE_EQUAL_EPSILON(A01,  0.0) &&
                HDOUBLE_EQUAL_EPSILON(A10,  0.0) &&
                HDOUBLE_EQUAL_EPSILON(A11, -1.0)   )
    {
        *po_pSlo = HBM_LLH;
    }
    else if (   HDOUBLE_EQUAL_EPSILON(A00, -1.0) &&
                HDOUBLE_EQUAL_EPSILON(A01,  0.0) &&
                HDOUBLE_EQUAL_EPSILON(A10,  0.0) &&
                HDOUBLE_EQUAL_EPSILON(A11, -1.0)   )
    {
        *po_pSlo = HBM_LRH;
    }
    else
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }


WRAPUP:
    if (isModelerAlloc)
        HBasicModeler_Destructor(&Modeler_Slo4);
    if (isMatrixAlloc)
        mat_destroy(matrix);

    HRET(Status);
}
/***************************************************************************
**  HBasicModeler_AddModeler
**
**  DESCRIPTION
**
**    This method combine two HBasicModeler object. The second is added to the
**    first modeler.
**
**  PARAMETERS
**
**    p_pHBasicModeler   <-> Pointer to a HBasicModeler object
**    pi_pModeler        ->  Modeler to add.
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin
****************************************************************************/
HSTATUS HBasicModeler_AddModeler     (HBasicModeler       *p_pHBasicModeler,
                                      HBasicModeler       *pi_pModeler)
{

    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;

    /*
    ** Scale Modeler Matrix
    */
    if (HISERROR(HSET(Status, sAddTransformation (&(THIS->m_pMatrix),
                                                  &((*pi_pModeler)->m_pMatrix)))))
    {
        goto WRAPUP;
    }


WRAPUP:

    HRET (Status);


}
/***************************************************************************
**  HBasicModeler_InvertModel
**
**  DESCRIPTION
**
**    This method compute the inverse of a HBasicModeler object
**
**  PARAMETERS
**
**    p_pHBasicModeler   <-> Pointer to a HBasicModeler object
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin
****************************************************************************/
HSTATUS HBasicModeler_InvertModel    (HBasicModeler       *p_pHBasicModeler)
{
    pMAT_  Matrix;
    int8_t isMatrixAlloc = FALSE;
    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;


    Matrix = mat_create(MATRIX_NB_ROW, MATRIX_NB_COL);

    if (Matrix == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }
    isMatrixAlloc = TRUE;

    mat_copyData(Matrix, THIS->m_pMatrix);

    /*
    ** Scale Modeler Matrix
    */
    if (HISERROR(HSET(Status, sInvertMatrix (Matrix, THIS->m_pMatrix))))
    {
        goto WRAPUP;
    }


WRAPUP:

    if (isMatrixAlloc)
        mat_destroy(Matrix);

    HRET (Status);


}
/***************************************************************************
**  HBasicModeler_ApplyToPoint
**
**  DESCRIPTION
**
**    This method transform an input point with the transformation matrix
**    of the input HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler   <-> Pointer to a HBasicModeler object
**    pi_pInputPt        ->  Input coordinate
**    po_pOutputPt       <-  Transformed coordinate
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin
****************************************************************************/
HSTATUS HBasicModeler_ApplyToPoint       (      HBasicModeler       *p_pHBasicModeler,
                                          const DCOORD              *pi_pInputPt,
                                                DCOORD              *po_pOutputPt)
{

    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;
    HPRECONDITION (pi_pInputPt);
    HPRECONDITION (po_pOutputPt);

    po_pOutputPt->x = mat_v(THIS->m_pMatrix,0,0) * pi_pInputPt->x +
                      mat_v(THIS->m_pMatrix,0,1) * pi_pInputPt->y +
                      mat_v(THIS->m_pMatrix,0,2) * pi_pInputPt->z +
                      mat_v(THIS->m_pMatrix,0,3);

    po_pOutputPt->y = mat_v(THIS->m_pMatrix,1,0) * pi_pInputPt->x +
                      mat_v(THIS->m_pMatrix,1,1) * pi_pInputPt->y +
                      mat_v(THIS->m_pMatrix,1,2) * pi_pInputPt->z +
                      mat_v(THIS->m_pMatrix,1,3);

    po_pOutputPt->z = mat_v(THIS->m_pMatrix,2,0) * pi_pInputPt->x +
                      mat_v(THIS->m_pMatrix,2,1) * pi_pInputPt->y +
                      mat_v(THIS->m_pMatrix,2,2) * pi_pInputPt->z +
                      mat_v(THIS->m_pMatrix,2,3);

    HRET (Status);
}

/***************************************************************************
**  HBasicModeler_BuildModel
**
**  DESCRIPTION
**
**    This method build a TRFMODEL from an HBasicModeler object
**
**  PARAMETERS
**
**    p_pHBasicModeler   <-> Pointer to a HBasicModeler object
**    pio_TransfoModel   <-  Output TRFMODEL
**
**  RETURN VALUE
**
**    H_SUCCESS   if successful
**    H_ERROR     otherwise
**
**  Stephane Poulin
****************************************************************************/
HSTATUS HBasicModeler_BuildModel         (const HBasicModeler *p_pHBasicModeler,
                                          TRFHANDLE            pio_TransfoModel)

{
    COEFFICIENTS_3D   ModelCoefs;

    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;

    if (HSUCCESS != trf_modelSetType (pio_TransfoModel, TRANSFO_MAT_2D))
    {
        HSET (Status, H_ERROR);
        goto WRAPUP;
    }


    ModelCoefs.nbCoef[XIND] = 3;
    ModelCoefs.nbCoef[YIND] = 3;
    ModelCoefs.nbCoef[ZIND] = 3;

    /*
    ** X coefs
    */
    ModelCoefs.coef[TRANSFO_DIRECT][XIND][1] = (double) mat_v(THIS->m_pMatrix, 0, 0);
    ModelCoefs.coef[TRANSFO_DIRECT][XIND][2] = (double) mat_v(THIS->m_pMatrix, 0, 1);
    ModelCoefs.coef[TRANSFO_DIRECT][XIND][0] = (double) mat_v(THIS->m_pMatrix, 0, 3);

    /*
    ** Y coefs
    */
    ModelCoefs.coef[TRANSFO_DIRECT][YIND][1] = (double) mat_v(THIS->m_pMatrix, 1, 0);
    ModelCoefs.coef[TRANSFO_DIRECT][YIND][2] = (double) mat_v(THIS->m_pMatrix, 1, 1);
    ModelCoefs.coef[TRANSFO_DIRECT][YIND][0] = (double) mat_v(THIS->m_pMatrix, 1, 3);

    /*
    ** Z coefs
    */
    ModelCoefs.coef[TRANSFO_DIRECT][ZIND][1] = (double) 0.0;
    ModelCoefs.coef[TRANSFO_DIRECT][ZIND][2] = (double) 0.0;
    ModelCoefs.coef[TRANSFO_DIRECT][ZIND][0] = (double) 0.0;

    if (HSUCCESS != trf_modelSetCoefficients   (pio_TransfoModel, &ModelCoefs))
    {   HSET (Status, H_ERROR);
        goto WRAPUP;
    }

WRAPUP:

    HRET (Status);
}

/*===========================================================================
**  HBasicModeler_GetTranslation
**
**  DESCRIPTION
**
**    This function return the current translation set in the transformation
**    matrix of the HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler  ->  Pointer to a HBasicModeler Object
**    po_pXOffset      <-   Pointer to a double that will recieve the X offset.
**    po_pYOffset      ->   Pointer to a double that will recieve the Y offset.
**
**  RETURN VALUE
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
**=========================================================================*/
HSTATUS HBasicModeler_GetTranslation(HBasicModeler  *p_pHBasicModeler,
                                     double        *po_pXOffset,
                                     double        *po_pYOffset)
{
    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;
    HPRECONDITION (po_pXOffset);
    HPRECONDITION (po_pYOffset);


    *po_pXOffset = mat_v(THIS->m_pMatrix, 0, 3);
    *po_pYOffset = mat_v(THIS->m_pMatrix, 1, 3);


    HRET (Status);
}
/*===========================================================================
**  HBasicModeler_GetScaling
**
**  DESCRIPTION
**
**    This function return the current X an Y scaling factors sets in the
**    transformation matrix of the HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler  ->  Pointer to a HBasicModeler Object
**    po_pXScale        <-  Pointer to a double that will recieve the X scale.
**    po_pYScale        ->  Pointer to a double that will recieve the Y scale.
**
**  RETURN VALUE
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
**=========================================================================*/
HSTATUS HBasicModeler_GetScaling    (HBasicModeler  *p_pHBasicModeler,
                                     double        *po_pXScale,
                                     double        *po_pYScale)
{
    double Angle;
    double A00, A01, A10, A11;

    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;
    HPRECONDITION (po_pXScale);
    HPRECONDITION (po_pYScale);


    A00 = mat_v(THIS->m_pMatrix, 0, 0);
    A01 = mat_v(THIS->m_pMatrix, 0, 1);
    A10 = mat_v(THIS->m_pMatrix, 1, 0);
    A11 = mat_v(THIS->m_pMatrix, 1, 1);

    /*
    ** Compute X scale
    */
    if(HBM_ISNOT_ZERO(A00) && HBM_ISNOT_ZERO(A10))
    {
        Angle = atan2(A10, A00);

        if (HBM_IS_ZERO_RADIANS(cos(Angle)))
            *po_pXScale = fabs(A10/sin(Angle));
        else
            *po_pXScale = fabs(A00/cos(Angle));
    }
    else
    {
        if (HBM_IS_ZERO(A00))
            *po_pXScale = fabs(A10);
        else
            *po_pXScale = fabs(A00);
    }

    /*
    ** Compute Y scale
    */
    if(HBM_ISNOT_ZERO(A01) && HBM_ISNOT_ZERO(A11))
    {
        Angle = atan2(-A01, A11);

        if(HBM_IS_ZERO_RADIANS(cos(Angle)))
            *po_pYScale = fabs(-A01/sin(Angle));
        else
            *po_pYScale = fabs(A11/cos(Angle));
    }
    else /* sin(a+b) == 0.0 or cos(a+b) == 0 */
    {
        if (HBM_IS_ZERO(A01))
            *po_pYScale = fabs(A11);
        else
            *po_pYScale = fabs(-A01);
    }

    HRET (Status);
}

/*===========================================================================
**  HBasicModeler_SetRotation
**
**  DESCRIPTION
**
**    This function sets the current rotation angle in radians in the
**    transformation matrix of the HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler  ->  Pointer to a HBasicModeler Object
**    po_Rotation       ->  Pointer to a double that hold the rotation angle
**                          in radians.
**
**  RETURN VALUE
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
**=========================================================================*/
HSTATUS HBasicModeler_SetRotation   (HBasicModeler  *p_pHBasicModeler,
                                     double         pi_Rotation)
{
    double ActualRotation;
    DCOORD  Origin;

    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;


    if(HISERROR(HSET(Status, HBasicModeler_GetRotation(&THIS, &ActualRotation))))
        goto WRAPUP;

    if(HISERROR(HSET(Status, HBasicModeler_GetTranslation(&THIS, &Origin.x, &Origin.y))))
        goto WRAPUP;
    Origin.z = 0.0;

    pi_Rotation -= ActualRotation;

    if(HISERROR(HSET(Status, HBasicModeler_AddRotation(&THIS, &Origin, pi_Rotation))))
        goto WRAPUP;

WRAPUP:
    HRET (Status);
}

/*===========================================================================
**  HBasicModeler_GetRotation
**
**  DESCRIPTION
**
**    This function return the current rotation angle in radians set in the
**    transformation matrix of the HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler  ->  Pointer to a HBasicModeler Object
**    po_pRotation      <-  Pointer to a double that will recieve the
**                          rotation angle.
**
**  RETURN VALUE
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
**=========================================================================*/
HSTATUS HBasicModeler_GetRotation   (HBasicModeler  *p_pHBasicModeler,
                                     double        *po_pRotation)
{
    double A00, A10;

    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;
    HPRECONDITION (po_pRotation);


    A00 = mat_v(THIS->m_pMatrix, 0, 0);
    A10 = mat_v(THIS->m_pMatrix, 1, 0);


    if(HBM_ISNOT_ZERO(A00) || HBM_ISNOT_ZERO(A10))
        *po_pRotation = atan2(A10,A00);
    else
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    /*
    ** Convert to a positive angle
    */
    while (*po_pRotation < 0.0)
           *po_pRotation = (2.0 * HBM_PI) + *po_pRotation;

    /*
    ** If angle is 2*PI return 0.0
    */
    if (HBM_IS_ZERO_RADIANS((2*PI)-(*po_pRotation)))
        *po_pRotation = 0.0;

WRAPUP:
    HRET (Status);
}

/*===========================================================================
**  HBasicModeler_SetAffinity
**
**  DESCRIPTION
**
**    This function sets the current Affinity angle in radians in the
**    transformation matrix of the HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler  ->  Pointer to a HBasicModeler Object
**    pi_Affinity       ->  Pointer to a double that hold the Affinity angle
**                          in radians.
**
**  RETURN VALUE
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
**=========================================================================*/
HSTATUS HBasicModeler_SetAffinity   (HBasicModeler  *p_pHBasicModeler,
                                     double         pi_Affinity)
{
    double A01, A11;
        double Angle;
        double Rotation_rad;
    double Sx, Sy;

    HDEF (Status, H_SUCCESS);
    HOBJECT_PRECONDITIONS;


        /*
        ** Do not allow affinity to be PI/2 or -PI/2
        */
        if (HBM_IS_ZERO_RADIANS(cos(pi_Affinity)))
        {
                HSET (Status, HERROR);
                goto WRAPUP;
        }

        if(HISERROR(HSET(Status, HBasicModeler_GetRotation(&THIS, &Rotation_rad))))
        goto WRAPUP;

    A01 = mat_v(THIS->m_pMatrix, 0, 1);
    A11 = mat_v(THIS->m_pMatrix, 1, 1);

    if(HBM_ISNOT_ZERO(A01)|| HBM_ISNOT_ZERO(A11))
            Angle = atan2(-A01, A11);
    else
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

        if(HBM_ISNOT_ZERO_RADIANS(sin(Angle)) && HBM_ISNOT_ZERO_RADIANS(cos(Angle)))
        {
                mat_v(THIS->m_pMatrix, 0, 1) = A01 / sin(Angle) * sin(Rotation_rad + pi_Affinity);
                mat_v(THIS->m_pMatrix, 1, 1) = A11 / cos(Angle) * cos(Rotation_rad + pi_Affinity);
        }
        else
        /*
        ** Rotation + Skew == 0 or 90 or 180 or 270
        */
        {
        if (HISERROR(HSET(Status, HBasicModeler_GetScaling(&THIS, &Sx, &Sy))))
            goto WRAPUP;

        if (HBM_IS_ZERO_RADIANS(sin(Angle)))
        {
                    mat_v(THIS->m_pMatrix, 0, 1) = -(Sy * sin(Rotation_rad + pi_Affinity));
                    mat_v(THIS->m_pMatrix, 1, 1) = A11 / cos(Angle) * cos(Rotation_rad + pi_Affinity);
        }
        else
        {
                    mat_v(THIS->m_pMatrix, 0, 1) = A01 / sin(Angle) * sin(Rotation_rad + pi_Affinity);
                    mat_v(THIS->m_pMatrix, 1, 1) = (Sy * cos(Rotation_rad + pi_Affinity));
        }
        }

WRAPUP:
    HRET (Status);
}

/*===========================================================================
**  HBasicModeler_GetAffinity
**
**  DESCRIPTION
**
**    This function return the current Affinity angle in radians set in the
**    transformation matrix of the HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler  ->  Pointer to a HBasicModeler Object
**    po_pRotation      <-  Pointer to a double that will recieve the
**                          Affinity angle.
**
**  RETURN VALUE
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
**=========================================================================*/
HSTATUS HBasicModeler_GetAffinity   (HBasicModeler  *p_pHBasicModeler,
                                     double        *po_pAffinity)
{
    double A00, A01, A11;
        double Rotation;

    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;
    HPRECONDITION (po_pAffinity);


    A00 = mat_v(THIS->m_pMatrix, 0, 0);
    A01 = mat_v(THIS->m_pMatrix, 0, 1);
    A11 = mat_v(THIS->m_pMatrix, 1, 1);

        if(HISERROR(HSET(Status, HBasicModeler_GetRotation(&THIS, &Rotation))))
                goto WRAPUP;

    if(HBM_ISNOT_ZERO(A01) || HBM_ISNOT_ZERO(A11))
        *po_pAffinity = atan2(-A01,A11) - Rotation;
    else
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    /*
    ** Convert to a positive angle
    */
    while (*po_pAffinity < 0.0)
           *po_pAffinity = (2.0 * HBM_PI) + *po_pAffinity;

    /*
    ** If angle is 2*PI return 0.0
    */
    if (HBM_IS_ZERO_RADIANS((2*PI)-(*po_pAffinity)))
        *po_pAffinity = 0.0;
WRAPUP:
    HRET (Status);
}

/*===========================================================================
**  HBasicModeler_GetMatrix
**
**  DESCRIPTION
**
**    This function return the current transformation matrix of the
**    HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler  -  Pointer to a HBasicModeler Object
**
**    po_pMatrix        -  Pointer to an allocated matrix.
**
**  RETURN VALUE
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
**=========================================================================*/
HSTATUS HBasicModeler_GetMatrix(HBasicModeler  *p_pHBasicModeler,
                                pMAT_           po_pMatrix)
{
    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;
    HPRECONDITION (po_pMatrix);

    if(HISERROR(HSET(Status, mat_copyData(po_pMatrix, THIS->m_pMatrix))))
        goto WRAPUP;

WRAPUP:
    HRET (Status);
}
/*===========================================================================
**  HBasicModeler_SetMatrix
**
**  DESCRIPTION
**
**    This function sets the current transformation matrix of the
**    HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler  -  Pointer to a HBasicModeler Object
**
**    po_pMatrix        -  Pointer to an allocated matrix.
**
**  RETURN VALUE
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
**=========================================================================*/
HSTATUS HBasicModeler_SetMatrix(HBasicModeler  *p_pHBasicModeler,
                                pMAT_           pi_pMatrix)
{
    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;
    HPRECONDITION (pi_pMatrix);

    if(HISERROR(HSET(Status, mat_copyData(THIS->m_pMatrix, pi_pMatrix))))
        goto WRAPUP;


WRAPUP:
    HRET (Status);
}
/*===========================================================================
**  HBasicModeler_AddTrfModel
**
**  DESCRIPTION
**
**    This function adds a transformation (TrfModel) to the transformation
**    matrix of the HBasicModeler object.
**
**  PARAMETERS
**
**    p_pHBasicModeler  <->  Pointer to a HBasicModeler Object
**    trfHandle          ->  Handle of the TrfModel
**
**  RETURN VALUE
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
**=========================================================================*/
HSTATUS HBasicModeler_AddTrfModel(HBasicModeler  *p_pHBasicModeler,
                                  TRFHANDLE       trfHandle)
{
    HBasicModeler   pTmpModeler;
    int8_t         modelerConstructed = FALSE;

    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;


    if (HISERROR(HSET(Status, HBasicModeler_ConstructorFromTrfModel (&pTmpModeler, trfHandle))))
        goto WRAPUP;

    modelerConstructed = TRUE;

    /*
    ** Combine both matrix
    */
    if(HISERROR(HSET(Status, sAddTransformation (&(THIS->m_pMatrix),
                                                 &(pTmpModeler->m_pMatrix)))))
        goto WRAPUP;


WRAPUP:

    if(modelerConstructed)
        HBasicModeler_Destructor(&pTmpModeler);

    HRET (Status);
}
/*===========================================================================
**  HBasicModeler_PrintMatrix
**
**  DESCRIPTION
**
**    This function print the current transformation matrix of the
**    HBasicModeler object.
**    For debugging purpose only!
**
**  PARAMETERS
**
**    p_pHBasicModeler  -  Pointer to a HBasicModeler Object
**
**
**  RETURN VALUE
**
**      H_SUCCESS   if successful
**
**  Stephane Poulin   FEB 1998  Original version
**=========================================================================*/
HSTATUS HBasicModeler_PrintMatrix(HBasicModeler  *p_pHBasicModeler)
{
    HDEF (Status, H_SUCCESS);

    HOBJECT_PRECONDITIONS;
    HPRECONDITION (THIS->m_pMatrix);

    mat_print(THIS->m_pMatrix);


    HRET (Status);
}

/*===========================================================================
**                        STATIC FUNCTION DEFINITION
**=========================================================================*/

/*===========================================================================
**  sAddTranslation
**
**
**      This function apply a translation to a transformation matrix
**
**  Parameters
**
**      matrix          Matrix to translate
**      pi_XOffset      Offset in X direction
**      pi_YOffset      Offset in X direction
**
**  Return Value
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
**=========================================================================*/
HSTATUS sAddTranslation  (pMAT_     *matrix,
                          double    pi_XOffset,
                          double    pi_YOffset)
{
    pMAT_ TranslationMatrix = MAT_NULL;

    HDEF (Status, H_SUCCESS);

    /*
    ** Allocate translation matrix
    */
    TranslationMatrix = mat_create(MATRIX_NB_ROW, MATRIX_NB_COL);
    if (TranslationMatrix == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    if(HISERROR(HSET(Status, sSetBasicTranslationMatrix (TranslationMatrix, pi_XOffset, pi_YOffset))))
        goto WRAPUP;

    if(HISERROR(HSET(Status, sAddTransformation(matrix, &TranslationMatrix))))
        goto WRAPUP;

WRAPUP:

    if(TranslationMatrix != MAT_NULL)
        mat_destroy(TranslationMatrix);

    HRET (Status);
}

/*===========================================================================
**  sAddScale
**
**
**      This function adds a scale to a transformation matrix
**
**  Parameters
**
**      matrix          Matrix to scale
**      pi_MirrorPoint  Point to scale about
**      pi_ScaleFactor  Scale factor
**
**
**  Return Value
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
**=========================================================================*/
HSTATUS sAddScale    (pMAT_         *matrix,
                      const DCOORD  *pi_pScalePoint,
                      double        pi_ScaleFactorX,
                      double        pi_ScaleFactorY)
{
    pMAT_ TransformMatrix = MAT_NULL;  /* Global Transformation matrix */
    pMAT_ ScalingMatrix   = MAT_NULL;  /* Simple Scaling matrix */

    HDEF (Status, H_SUCCESS);


    /*
    ** Create the global transformation matrix and
    ** initialize it with identity matrix
    */
    TransformMatrix = mat_create(MATRIX_NB_ROW, MATRIX_NB_COL);
    if (TransformMatrix == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    if(HISERROR(HSET(Status, mat_setIdentity(TransformMatrix))))
        goto WRAPUP;

    /*
    ** Create a simple scaling matrix.
    */
    ScalingMatrix = mat_create(MATRIX_NB_ROW, MATRIX_NB_COL);
    if (ScalingMatrix == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    /*
    ** Set basic Scaling matrix
    */
    if(HISERROR(HSET(Status, sSetBasicScalingMatrix (ScalingMatrix,
                                                     pi_ScaleFactorX,
                                                     pi_ScaleFactorY))))
        goto WRAPUP;

    /*
    ** Translate the global transformation matrix
    */
    if (HISERROR(HSET(Status, sAddTranslation(&TransformMatrix, -(pi_pScalePoint->x), -(pi_pScalePoint->y)))))
        goto WRAPUP;

    /*
    ** Scale the global transformation matrix
    */
    if(HISERROR(HSET(Status, sAddTransformation(&TransformMatrix, &ScalingMatrix))))
        goto WRAPUP;


    /*
    ** Translate back the global transformation matrix
    */
    if (HISERROR(HSET(Status, sAddTranslation(&TransformMatrix, (pi_pScalePoint->x), (pi_pScalePoint->y)))))
        goto WRAPUP;

    /*
    ** Apply global transformation matrix to the basic modeler matrix
    */
    if(HISERROR(HSET(Status, sAddTransformation(matrix, &TransformMatrix))))
        goto WRAPUP;


WRAPUP:
    if(TransformMatrix != MAT_NULL)
        mat_destroy(TransformMatrix);
    if(ScalingMatrix != MAT_NULL)
        mat_destroy(ScalingMatrix);

    HRET (Status);
}

/*===========================================================================
**  sAddRotation
**
**
**      This function apply a rotation to a transformation matrix
**
**  Parameters
**
**      matrix          Matrix to rotate
**      pi_MirrorPoint  Point to rotate about
**      pi_Rotation     Rotation angle in radians
**
**
**  Return Value
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
**=========================================================================*/
HSTATUS sAddRotation (pMAT_         *matrix,
                      const DCOORD  *pi_pRotationPoint,
                      double        pi_Rotation)
{
    pMAT_ TransformMatrix = MAT_NULL;  /* Global rotation matrix */
    pMAT_ RotationMatrix  = MAT_NULL;  /* Simple rotation matrix */

    HDEF (Status, H_SUCCESS);

    /*
    ** Create a global transformation matrix and
    ** initialize it with identity matrix
    */
    TransformMatrix = mat_create(MATRIX_NB_ROW, MATRIX_NB_COL);
    if (TransformMatrix == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    if(HISERROR(HSET(Status, mat_setIdentity(TransformMatrix))))
        goto WRAPUP;

    /*
    ** Create and set a simple rotation matrix.
    */
    RotationMatrix = mat_create(MATRIX_NB_ROW, MATRIX_NB_COL);
    if (RotationMatrix == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    if(HISERROR(HSET(Status, sSetBasicRotationMatrix(RotationMatrix, pi_Rotation))))
        goto WRAPUP;

    /*
    ** Translate the global transformation matrix
    */
    if (HISERROR(HSET(Status, sAddTranslation(&TransformMatrix, -(pi_pRotationPoint->x), -(pi_pRotationPoint->y)))))
        goto WRAPUP;

    /*
    ** Rotate the global transformation matrix
    */
    if(HISERROR(HSET(Status, sAddTransformation(&TransformMatrix, &RotationMatrix))))
        goto WRAPUP;


    /*
    ** Translate back the global transformation matrix
    */
    if (HISERROR(HSET(Status, sAddTranslation(&TransformMatrix, (pi_pRotationPoint->x), (pi_pRotationPoint->y)))))
        goto WRAPUP;

    /*
    ** Apply global transformation matrix to the basic modeler matrix
    */
    if(HISERROR(HSET(Status, sAddTransformation(matrix, &TransformMatrix))))
        goto WRAPUP;


WRAPUP:
    if(TransformMatrix != MAT_NULL)
        mat_destroy(TransformMatrix);
    if(RotationMatrix != MAT_NULL)
        mat_destroy(RotationMatrix);

    HRET (Status);
}

/*===========================================================================
**  sAddMirror
**
**
**      This function apply a mirror to a transformation matrix
**
**  Parameters
**
**      matrix          Matrix to mirror
**      pi_MirrorPoint  Point to mirror about
**      mirrorType      Kind of mirror
**                          - MIRROR_VERTICAL
**                          - MIRROR_HORIZONTAL
**                          - MIRROR_DOWNUP
**                          - MIRROR_UPDOWN
**
**  Return Value
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
**=========================================================================*/
HSTATUS sAddMirror    (pMAT_        *matrix,
                       const DCOORD *pi_MirrorPoint,
                       int32_t       pi_MirrorType)
{
    pMAT_ MirrorMatrix    = MAT_NULL;    /* Simple mirror matrix */
    pMAT_ TransformMatrix = MAT_NULL;    /* Global mirror matrix */

    HDEF (Status, H_SUCCESS);

    /*
    ** Create a global transformation matrix and
    ** initialize it with identity matrix
    */
    TransformMatrix = mat_create(MATRIX_NB_ROW, MATRIX_NB_COL);
    if (TransformMatrix == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    if(HISERROR(HSET(Status, mat_setIdentity(TransformMatrix))))
        goto WRAPUP;

    /*
    ** Create set a simple Mirror matrix.
    */
    MirrorMatrix = mat_create(MATRIX_NB_ROW, MATRIX_NB_COL);
    if (MirrorMatrix == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }
    if (HISERROR(HSET(Status, sSetBasicMirrorMatrix(MirrorMatrix, pi_MirrorType))))
        goto WRAPUP;

    /*
    ** Translate the global transformation matrix
    */
    if (HISERROR(HSET(Status, sAddTranslation(&TransformMatrix, -(pi_MirrorPoint->x), -(pi_MirrorPoint->y)))))
        goto WRAPUP;

    /*
    ** Scale the global transformation matrix
    */
    if(HISERROR(HSET(Status, sAddTransformation(&TransformMatrix, &MirrorMatrix))))
        goto WRAPUP;


    /*
    ** Translate back the global transformation matrix
    */
    if (HISERROR(HSET(Status, sAddTranslation(&TransformMatrix, (pi_MirrorPoint->x), (pi_MirrorPoint->y)))))
        goto WRAPUP;

    /*
    ** Apply global transformation matrix to the basic modeler matrix
    */
    if(HISERROR(HSET(Status, sAddTransformation(matrix, &TransformMatrix))))
        goto WRAPUP;


WRAPUP:
    if(TransformMatrix != MAT_NULL)
        mat_destroy(TransformMatrix);
    if(MirrorMatrix != MAT_NULL)
        mat_destroy(MirrorMatrix);

    HRET (Status);
}


/*===========================================================================
**  sSetBasicMirrorMatrix
**
**
**      This function return a basic 4X4 mirror matrix
**
**  Parameters
**
**      matrix      ->  Pointer to a matrix pointer
**      mirrorType  ->  Kind of mirror needed
**                          - MIRROR_VERTICAL
**                          - MIRROR_HORIZONTAL
**                          - MIRROR_DOWNUP
**                          - MIRROR_UPDOWN
**
**  Return Value
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   05/11/1997  Original version
**=========================================================================*/
HSTATUS sSetBasicMirrorMatrix(pMAT_ matrix, int32_t mirrorType)
{
    HDEF (Status, H_SUCCESS);

    if(matrix == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    if (HISERROR(HSET(Status, mat_setIdentity(matrix))))
        goto WRAPUP;

    switch (mirrorType)
    {
    case MIRROR_VERTICAL:
        mat_v((matrix), 0, 0) = -1.0;
        break;

    case MIRROR_HORIZONTAL:
        mat_v((matrix), 1, 1) = -1.0;
        break;

    case MIRROR_DOWNUP:
        mat_v((matrix), 0, 0) = 0.0;
        mat_v((matrix), 0, 1) = 1.0;
        mat_v((matrix), 1, 0) = 1.0;
        mat_v((matrix), 1, 1) = 0.0;
        break;

    case MIRROR_UPDOWN:
        mat_v((matrix), 0, 0) =  0.0;
        mat_v((matrix), 0, 1) = -1.0;
        mat_v((matrix), 1, 0) = -1.0;
        mat_v((matrix), 1, 1) =  0.0;
        break;
    default:
        HSET(Status, HERROR);
        goto WRAPUP;
        break;
    }

WRAPUP:

    HRET (Status);
}

/*===========================================================================
**  sSetBasicTranslationMatrix
**
**  DESCRIPTION
**
**    Set a basic Translation matrix.
**
**  PARAMETERS
**
**      ScalingMatrix   -> Matrix
**      pi_XOffset      -> X offset
**      pi_YOffset      -> Y offset
**
**  Return Value
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   13/11/1997  Original version
**=========================================================================*/
HSTATUS sSetBasicTranslationMatrix (pMAT_      TransformMatrix,
                                    double    pi_XOffset,
                                    double    pi_YOffset)
{
    HDEF (Status, H_SUCCESS);

    if(TransformMatrix == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    if(HISERROR(HSET(Status, mat_setIdentity(TransformMatrix))))
        goto WRAPUP;

    mat_v((TransformMatrix), 0, 3) = pi_XOffset;
    mat_v((TransformMatrix), 1, 3) = pi_YOffset;

WRAPUP:
    HRET(Status);
}

/*===========================================================================
**  sSetBasicRotationMatrix
**
**  DESCRIPTION
**
**    Set a basic Rotation matrix.
**
**  PARAMETERS
**
**      ScalingMatrix   -> Matrix
**      pi_Rotation     -> Rotation angle in radians
**
**  Return Value
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   13/11/1997  Original version
**=========================================================================*/
HSTATUS sSetBasicRotationMatrix(pMAT_          RMatrix,
                                double        pi_Rotation)
{
    HDEF (Status, H_SUCCESS);

    if(RMatrix == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    if (HISERROR(HSET(Status, mat_setIdentity(RMatrix))))
        goto WRAPUP;

    mat_v((RMatrix), 0, 0) =  cos(pi_Rotation);
    mat_v((RMatrix), 0, 1) = -sin(pi_Rotation);
    mat_v((RMatrix), 1, 0) =  sin(pi_Rotation);
    mat_v((RMatrix), 1, 1) =  cos(pi_Rotation);


WRAPUP:
    HRET(Status);
}

/*===========================================================================
**  sSetBasicScalingMatrix
**
**  DESCRIPTION
**
**    Set a basic scaling matrix.
**
**  PARAMETERS
**
**      ScalingMatrix        -> Matrix
**      pi_ScaleFactor[X|Y]  -> Scaling factor
**
**  Return Value
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   13/11/1997  Original version
**=========================================================================*/
HSTATUS sSetBasicScalingMatrix (pMAT_          ScalingMatrix,
                                double        pi_ScaleFactorX,
                                double        pi_ScaleFactorY)
{
    HDEF (Status, H_SUCCESS);

    if(ScalingMatrix == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    if (HISERROR(HSET(Status, mat_setIdentity(ScalingMatrix))))
        goto WRAPUP;

    mat_v((ScalingMatrix), 0, 0) = pi_ScaleFactorX;
    mat_v((ScalingMatrix), 1, 1) = pi_ScaleFactorY;

WRAPUP:
    HRET(Status);
}

/*===========================================================================
**  sSetBasicAffinityMatrix
**
**  DESCRIPTION
**
**    Set a basic Affinity matrix in the X direction.
**
**  PARAMETERS
**
**      ScalingMatrix   -> Matrix
**      pi_Affinity     -> Affinity angle in radians
**
**  Return Value
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   13/11/1997  Original version
**=========================================================================*/
HSTATUS sSetBasicAffinityMatrix(pMAT_          SHMatrix,
                                double        pi_Affinity)
{
    HDEF (Status, H_SUCCESS);

    if(SHMatrix == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

        /*
        ** Do not allow affinity to be PI/2 or -PI/2
        */
        if (HBM_IS_ZERO_RADIANS(cos(pi_Affinity)))
        {
                HSET (Status, HERROR);
                goto WRAPUP;
        }

    if (HISERROR(HSET(Status, mat_setIdentity(SHMatrix))))
        goto WRAPUP;

    mat_v((SHMatrix), 0, 1) = -sin(pi_Affinity);
    mat_v((SHMatrix), 1, 1) =  cos(pi_Affinity);


WRAPUP:
    HRET(Status);
}

/*===========================================================================
**  sAddTransformation
**
**  DESCRIPTION
**
**    Add transformation stored in matrix B to the matrix A
**
**    A = B * A
**
**  PARAMETERS
**
**      A               Pointer to a matrix to transform
**
**      B               Pointer to a transformation matrix
**                      B is applied to A
**
**  Return Value
**
**      H_SUCCESS   if successful
**      H_ERROR     otherwise
**
**  Stephane Poulin   13/11/1997  Original version
**=========================================================================*/
HSTATUS sAddTransformation (pMAT_ *A, pMAT_ *B)
{
    pMAT_ tmpMat = MAT_NULL;

    HDEF (Status, H_SUCCESS);


    tmpMat = mat_dmxmul(*B, *A, MULT_AXB);
    if (tmpMat == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    if(HISERROR(HSET(Status, mat_copyData(*A, tmpMat))))
        goto WRAPUP;

WRAPUP:
    if(tmpMat != MAT_NULL)
        mat_destroy(tmpMat);

    HRET(Status);
}
/*===========================================================================
**  sInvertMatrix
**
**  DESCRIPTION
**
**    Calculate the inverse of a 4 X 4 matrix.
**     -1
**    A   = __1__ adjoint(A)
**          DET(A)
**
**  PARAMETERS
**
**      pi_pInputMatrix  - Input 4 X 4 matrix.
**      po_pOutputMatrix - Output 4 X 4 matrix.
**
**  Return Value
**
**      double   The determinant of the marix.
**
**  Stephane Poulin   FEB 1998  Original version
**=========================================================================*/
HSTATUS sInvertMatrix(pMAT_ pi_pInputMatrix, pMAT_ po_pOutputMatrix)
{
    int32_t  i, j;
    double  det;
    pMAT_    in, out;

    HDEF(Status, HSUCCESS);

    HPRECONDITION (pi_pInputMatrix);
    HPRECONDITION (po_pOutputMatrix);

    in  = pi_pInputMatrix;
    out = po_pOutputMatrix;

    if(HISERROR(HSET(Status, sAdjoint(in, out))))
        goto WRAPUP;

    det = det4x4(in);

    /*
    ** Check if Determinant is Zero
    */
    if(HDOUBLE_EQUAL_EPSILON(fabs(det), 0.0))
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    for(i=0; i<4; i++)
        for(j=0; j<4; j++)
            mat_v(out,i,j) = mat_v(out,i,j)/det;

WRAPUP:
    HRET(Status);
}
/*===========================================================================
**  sAdjoint
**
**  DESCRIPTION
**
**    Calculate the adjoint of a 4 X 4 matrix.
**    An adjoint matrix is simply the transpose of it's cofactors.
**
**  PARAMETERS
**
**      in  - Input 4 X 4 matrix.
**      out - Output 4 X 4 matrix.
**
**  Return Value
**
**      HSUCCESS
**
**  Stephane Poulin   FEB 1998  Original version
**=========================================================================*/
HSTATUS sAdjoint(pMAT_ in, pMAT_ out)
{
    double a1,a2,a3,a4,b1,b2,b3,b4;
    double c1,c2,c3,c4,d1,d2,d3,d4;

    a1 = mat_v(in,0,0);
    b1 = mat_v(in,0,1);
    c1 = mat_v(in,0,2);
    d1 = mat_v(in,0,3);

    a2 = mat_v(in,1,0);
    b2 = mat_v(in,1,1);
    c2 = mat_v(in,1,2);
    d2 = mat_v(in,1,3);

    a3 = mat_v(in,2,0);
    b3 = mat_v(in,2,1);
    c3 = mat_v(in,2,2);
    d3 = mat_v(in,2,3);

    a4 = mat_v(in,3,0);
    b4 = mat_v(in,3,1);
    c4 = mat_v(in,3,2);
    d4 = mat_v(in,3,3);

    mat_v(out,0,0) =   det3x3(b2,b3,b4,c2,c3,c4,d2,d3,d4);
    mat_v(out,1,0) = - det3x3(a2,a3,a4,c2,c3,c4,d2,d3,d4);
    mat_v(out,2,0) =   det3x3(a2,a3,a4,b2,b3,b4,d2,d3,d4);
    mat_v(out,3,0) = - det3x3(a2,a3,a4,b2,b3,b4,c2,c3,c4);

    mat_v(out,0,1) = - det3x3(b1,b3,b4,c1,c3,c4,d1,d3,d4);
    mat_v(out,1,1) =   det3x3(a1,a3,a4,c1,c3,c4,d1,d3,d4);
    mat_v(out,2,1) = - det3x3(a1,a3,a4,b1,b3,b4,d1,d3,d4);
    mat_v(out,3,1) =   det3x3(a1,a3,a4,b1,b3,b4,c1,c3,c4);

    mat_v(out,0,2) =   det3x3(b1,b2,b4,c1,c2,c4,d1,d2,d4);
    mat_v(out,1,2) = - det3x3(a1,a2,a4,c1,c2,c4,d1,d2,d4);
    mat_v(out,2,2) =   det3x3(a1,a2,a4,b1,b2,b4,d1,d2,d4);
    mat_v(out,3,2) = - det3x3(a1,a2,a4,b1,b2,b4,c1,c2,c4);

    mat_v(out,0,3) = - det3x3(b1,b2,b3,c1,c2,c3,d1,d2,d3);
    mat_v(out,1,3) =   det3x3(a1,a2,a3,c1,c2,c3,d1,d2,d3);
    mat_v(out,2,3) = - det3x3(a1,a2,a3,b1,b2,b3,d1,d2,d3);
    mat_v(out,3,3) =   det3x3(a1,a2,a3,b1,b2,b3,c1,c2,c3);

    HRET(HSUCCESS);
}
/*===========================================================================
**  det4x4
**
**  DESCRIPTION
**
**    Calculate the determinant of a 4 X 4 matrix.
**
**  PARAMETERS
**
**      mat - pMAT_ structure that contain a 4 X 4 matrix.
**
**
**  Return Value
**
**      double   The determinant of the marix.
**
**  Stephane Poulin   FEB 1998  Original version
**=========================================================================*/
double det4x4(pMAT_ mat)
{
    double ans;
    double a1,a2,a3,a4,b1,b2,b3,b4;
    double c1,c2,c3,c4,d1,d2,d3,d4;


    a1 = mat_v(mat,0,0);
    b1 = mat_v(mat,0,1);
    c1 = mat_v(mat,0,2);
    d1 = mat_v(mat,0,3);

    a2 = mat_v(mat,1,0);
    b2 = mat_v(mat,1,1);
    c2 = mat_v(mat,1,2);
    d2 = mat_v(mat,1,3);

    a3 = mat_v(mat,2,0);
    b3 = mat_v(mat,2,1);
    c3 = mat_v(mat,2,2);
    d3 = mat_v(mat,2,3);

    a4 = mat_v(mat,3,0);
    b4 = mat_v(mat,3,1);
    c4 = mat_v(mat,3,2);
    d4 = mat_v(mat,3,3);

    ans = a1 * det3x3(b2, b3, b4, c2, c3, c4, d2, d3, d4) -
          b1 * det3x3(a2, a3, a4, c2, c3, c4, d2, d3, d4) +
          c1 * det3x3(a2, a3, a4, b2, b3, b4, d2, d3, d4) -
          d1 * det3x3(a2, a3, a4, b2, b3, b4, c2, c3, c4);

    return(ans);
}
/*===========================================================================
**  det3x3
**
**  DESCRIPTION
**
**    Calculate the determinant of a 3 X 3 matrix.
**
**  PARAMETERS
**
**      double
**
**      |a1 a2 a3|
**      |b1 b2 b3|
**      |c1 c2 c3|
**
**  Return Value
**
**      double   The determinant of the marix.
**
**  Stephane Poulin   FEB 1998  Original version
**=========================================================================*/
double det3x3(double a1, double a2, double a3,
               double b1, double b2, double b3,
               double c1, double c2, double c3)
{
    double ans;
    double det2x2(double a, double b, double c, double d);

    ans = a1 * det2x2(b2, b3, c2, c3) -
          b1 * det2x2(a2, a3, c2, c3) +
          c1 * det2x2(a2, a3, b2, b3);

    return (ans);

}
/*===========================================================================
**  det2x2
**
**  DESCRIPTION
**
**    Calculate the determinant of a 2 X 2 matrix.
**
**  PARAMETERS
**
**      |a b|
**      |c d|
**
**      a   -> Matrix value (0,0)
**      b   -> Matrix value (0,1)
**      c   -> Matrix value (1,0)
**      d   -> Matrix value (1,1)
**
**  Return Value
**
**      double   The determinant of the marix.
**
**  Stephane Poulin   FEB 1998  Original version
**=========================================================================*/
double det2x2(double a, double b, double c, double d)
{
    double ans;

    ans = a * d - b * c;

    return (ans);
}