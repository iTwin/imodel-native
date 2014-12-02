/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/MatrixElements.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

// matrix double data element transform types (see mselems.h for more defines):
#define MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_MASK   0x00F
#define MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_MASK     0x0F0   // doubles per matrix struct
#define MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_1        0x010
#define MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_2        0x020
#define MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_3        0x030
#define MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_4        0x040

#define MATRIX_DATA_ELM_TRANSFORM_TYPE_UNIT_MASK    0x100   // preserve [co]bvector unit length

// matrix int data transform types (see mselems.h for more defines):
#define MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_MASK       0x000F

#define MATRIX_DATA_ELM_INDEX_TYPE_BASE_MASK        0x00F0
#define MATRIX_DATA_ELM_INDEX_TYPE_BASE_ZERO        0x0010  // 0-based data
#define MATRIX_DATA_ELM_INDEX_TYPE_BASE_ONE         0x0020  // 1-based data

#define MATRIX_DATA_ELM_INDEX_TYPE_PAD_MASK         0x0F00
#define MATRIX_DATA_ELM_INDEX_TYPE_PAD_NONE         0x0000  // only valid with no/fixed blocking
#define MATRIX_DATA_ELM_INDEX_TYPE_PAD_ZERO         0x0100  //  0 = block padder/terminator
#define MATRIX_DATA_ELM_INDEX_TYPE_PAD_MINUSONE     0x0200  // -1 = block padder/terminator

#define MATRIX_DATA_ELM_INDEX_TYPE_SIGNED_MASK      0x1000  // may have negative data

/*---------------------------------------------------------------------------------**//**
* @description Validate the transform type for a matrix double data element.
*
* @param transformType  IN      code to validate
* @return SUCCESS if valid transform type code
* @group    "Matrix Elements"
* @bsimethod                                    DavidAssaf      12/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MatrixHeaderUtils::ValidateDoubleTransformType
(
int transformType
)
    {
    int coordType = transformType & MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_MASK;
    int dim = transformType & MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_MASK;

    /* check possible values: use #defines in mselems.h and elements.h */
    switch (coordType)
        {
        /* no transform type */
        case MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_NONE:
            return SUCCESS;
            break;

        case MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_POINT:
        case MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_VECTOR:
        case MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_COVECTOR:
            break;

        default:
            return ERROR;
            break;
        }

    switch (dim)
        {
        case MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_1:
        case MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_2:
        case MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_3:
        case MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_4:
            break;

        default:
            return ERROR;
            break;
        }

    /* no invalid combinations */

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description Construct the transform type for a matrix double data element.
* <P>
* Pass MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_NONE for coordType if data is not to be
*       transformed (in this case, the other inputs are ignored).
* @param pTransformType OUT     code to encipher
* @param coordType      IN      MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_*
* @param dimension      IN      (1, 2, 3 or 4) dimension of data
* @param bNormalized    IN      true if unit length is preserved under transformation
* @return SUCCESS if valid transform type code
* @group    "Matrix Elements"
* @bsimethod                                    DavidAssaf      12/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MatrixHeaderUtils::EncodeTransformType
(
int         *pTransformType,
int         coordType,
int         dimension,
bool        bNormalized
)
    {
    if (!pTransformType)
        return ERROR;

    *pTransformType = (coordType & MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_MASK);

    if (coordType != MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_NONE)
        {
        switch (dimension)
            {
            case 1:
                *pTransformType |= MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_1;
                break;

            case 2:
                *pTransformType |= MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_2;
                break;

            case 3:
                *pTransformType |= MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_3;
                break;

            case 4:
                *pTransformType |= MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_4;
                break;
            }

        if (bNormalized)
            *pTransformType |= MATRIX_DATA_ELM_TRANSFORM_TYPE_UNIT_MASK;
        }

    return ValidateDoubleTransformType (*pTransformType);
    }

/*---------------------------------------------------------------------------------**//**
* @description Return the information encoded in the transform type of a matrix double
*       data element.  NULL may be supplied for any combination of output parameters.
* <P>
* If pCoordType returns (or would return, if it were not NULL) with the value
*       MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_NONE, then the other outputs are unfilled.
*       This indicates the matrix data is not transformable.
* @param transformType  IN      code to decipher
* @param pCoordType     OUT     MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_*
* @param pDimension     OUT     (1, 2, 3 or 4) dimension of data
* @param pbNormalized   OUT     true if unit length is preserved under transformation
* @return SUCCESS if valid transform type code
* @group    "Matrix Elements"
* @bsimethod                                    DavidAssaf      12/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MatrixHeaderUtils::DecodeTransformType
(
int         transformType,
int         *pCoordType,
int         *pDimension,
bool        *pbNormalized
)
    {
    int coordType = transformType & MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_MASK;

    if (SUCCESS != ValidateDoubleTransformType (transformType))
        return ERROR;

    if (pCoordType)
        *pCoordType = coordType;

    /* code signifies "do not transform these doubles" */
    if (coordType == MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_NONE)
        return SUCCESS;

    if (pDimension)
        {
        switch (transformType & MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_MASK)
            {
            case MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_1:
                *pDimension = 1;
                break;

            case MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_2:
                *pDimension = 2;
                break;

            case MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_3:
                *pDimension = 3;
                break;

            case MATRIX_DATA_ELM_TRANSFORM_TYPE_DIM_4:
                *pDimension = 4;
                break;
            }
        }

    if (pbNormalized)
        {
        *pbNormalized = (transformType & MATRIX_DATA_ELM_TRANSFORM_TYPE_UNIT_MASK)
                        ? true : false;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description Transform the given array of (scalar) doubles according to the transform type code.
*
* @param pBuf           IN OUT  array to transform
* @param count          IN      # points in array
* @param pTransform     IN      transform
* @param transformType  IN      indicates how structs are transformed
* @return SUCCESS if the transform was successful, or if it was not required
* @group    "Matrix Elements"
* @see mdlMatrixDoubleDataElement_transform
* @bsimethod                                    DavidAssaf      02/04
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MatrixHeaderUtils::TransformScalarBuffer
(
        double*     pBuf,
        int         count,
const   Transform*  pTransform,
        int         transformType
)
    {
    int             i;
    int             coordType, dim;
    bool            bNormalized;
    double          scale, translate;
    BentleyStatus       status = ERROR;

    // validate input
    if (!pBuf || SUCCESS != DecodeTransformType (transformType, &coordType, &dim, &bNormalized))
        return ERROR;

    // no transform => noop
    if (!pTransform || coordType == MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_NONE)
        return SUCCESS;

    // sanity check
    if (dim != 1)
        return ERROR;

    // scale by M[0,0], not by magnitude of the first column!
    scale = bsiRotMatrix_getMatrixComponentByRowAndColumn (pTransform, 0, 0);
    translate = bsiRotMatrix_getTranslationComponent (pTransform, 0);

    switch (coordType)
        {
        case MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_POINT:
            // [M t] p = s p + t
            for (i = 0; i < count; i++)
                pBuf[i] = scale * pBuf[i] + translate;
            status = SUCCESS;
            break;

        case MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_VECTOR:
            // [M t] p = s p
            for (i = 0; i < count; i++)
                pBuf[i] = scale * pBuf[i];

            if (bNormalized)
                for (i = 0; i < count; i++)
                    if (pBuf[i])
                        pBuf[i] = (pBuf[i] > 0.0) ? 1.0 : -1.0;

            status = SUCCESS;
            break;

        case MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_COVECTOR:
            // [M t] p = 1/s p (rationalized by 2D/3D method)
            if (scale)
                {
                scale = 1.0 / scale;

                for (i = 0; i < count; i++)
                    pBuf[i] = scale * pBuf[i];

                if (bNormalized)
                    for (i = 0; i < count; i++)
                        if (pBuf[i])
                            pBuf[i] = (pBuf[i] > 0.0) ? 1.0 : -1.0;
                }

            status = SUCCESS;
            break;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @description Transform the given array of DPoint2d according to the transform type code.
*
* @param pBuf           IN OUT  array to transform
* @param count          IN      # points in array
* @param pTransform     IN      transform
* @param transformType  IN      indicates how structs are transformed
* @return SUCCESS if the transform was successful, or if it was not required
* @group    "Matrix Elements"
* @see mdlMatrixDoubleDataElement_transform
* @bsimethod                                    DavidAssaf      11/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MatrixHeaderUtils::TransformDPoint2dBuffer
(
        DPoint2d    *pBuf,
        int         count,
const   Transform   *pTransform,
        int         transformType
)
    {
    int             i;
    int             coordType, dim;
    bool            bNormalized;
    RotMatrix       matrix;
    BentleyStatus       status = ERROR;

    /* validate input */
    if (   !pBuf
        || SUCCESS != DecodeTransformType (transformType, &coordType, &dim, &bNormalized)
        )
        return ERROR;

    /* no transform => noop */
    if (!pTransform || coordType == MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_NONE)
        return SUCCESS;

    /* sanity check */
    if (dim != 2)
        return ERROR;

    switch (coordType)
        {
        case MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_POINT:
            /* [M t] p = M p + t */
            bsiTransform_multiplyDPoint2dArray (pTransform, pBuf, pBuf, count);
            status = SUCCESS;
            break;

        case MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_VECTOR:
            /* [M t] p = M p */
            bsiTransform_getMatrix (pTransform, &matrix);
            bsiRotMatrix_multiplyDPoint2dArray (&matrix, pBuf, pBuf, count);
            if (bNormalized)
                {
                for (i = 0; i < count; i++)
                    bsiDPoint2d_normalize (pBuf + i);
                }
            status = SUCCESS;
            break;

        case MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_COVECTOR:
            /* [M t] p = M-^ p => 0 = p^ q = (M-^ p)^ M q => orthog preserved */
            bsiTransform_getMatrix (pTransform, &matrix);

            // explicitly initialize entries with z-effect so that they don't factor into inverse
            bsiRotMatrix_setComponentByRowAndColumn (&matrix, 0, 2, 0.0);
            bsiRotMatrix_setComponentByRowAndColumn (&matrix, 1, 2, 0.0);
            bsiRotMatrix_setComponentByRowAndColumn (&matrix, 2, 0, 0.0);
            bsiRotMatrix_setComponentByRowAndColumn (&matrix, 2, 1, 0.0);
            bsiRotMatrix_setComponentByRowAndColumn (&matrix, 2, 2, 1.0);

            if (bsiRotMatrix_invertInPlace (&matrix))
                {
                bsiRotMatrix_transposeInPlace (&matrix);
                bsiRotMatrix_multiplyDPoint2dArray (&matrix, pBuf, pBuf, count);
                if (bNormalized)
                    {
                    for (i = 0; i < count; i++)
                        bsiDPoint2d_normalize (pBuf + i);
                    }
                status = SUCCESS;
                }
            break;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @description Transform the given array of DPoint3d according to the transform type code.
*
* @param pBuf           IN OUT  array to transform
* @param count          IN      # points in array
* @param pTransform     IN      transform
* @param transformType  IN      indicates how structs are transformed
* @return SUCCESS if the transform was successful, or if it was not required
* @group    "Matrix Elements"
* @see mdlMatrixDoubleDataElement_transform
* @bsimethod                                    DavidAssaf      11/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MatrixHeaderUtils::TransformDPoint3dBuffer
(
        DPoint3d    *pBuf,
        int         count,
const   Transform   *pTransform,
        int         transformType
)
    {
    int             i;
    int             coordType, dim;
    bool            bNormalized;
    RotMatrix       matrix;
    BentleyStatus       status = ERROR;

    /* validate input */
    if (   !pBuf
        || SUCCESS != DecodeTransformType (transformType, &coordType, &dim, &bNormalized)
        )
        return ERROR;

    /* no transform => noop */
    if (!pTransform || coordType == MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_NONE)
        return SUCCESS;

    /* sanity check */
    if (dim != 3)
        return ERROR;

    /* keep cases up to date with defines in mselems.h */
    switch (coordType)
        {
        case MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_POINT:
            /* [M t] p = M p + t */
            bsiTransform_multiplyDPoint3dArrayInPlace (pTransform, pBuf, count);
            status = SUCCESS;
            break;

        case MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_VECTOR:
            /* [M t] p = M p */
            bsiTransform_getMatrix (pTransform, &matrix);
            bsiRotMatrix_multiplyDPoint3dArray (&matrix, pBuf, pBuf, count);
            if (bNormalized)
                {
                for (i = 0; i < count; i++)
                    bsiDPoint3d_normalizeInPlace (pBuf + i);
                }
            status = SUCCESS;
            break;

        case MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_COVECTOR:
            /* [M t] p = M-^ p => 0 = p^ q = (M-^ p)^ M q => orthog preserved */
            bsiTransform_getMatrix (pTransform, &matrix);
            if (bsiRotMatrix_invertInPlace (&matrix))
                {
                bsiRotMatrix_transposeInPlace (&matrix);
                bsiRotMatrix_multiplyDPoint3dArray (&matrix, pBuf, pBuf, count);
                if (bNormalized)
                    {
                    for (i = 0; i < count; i++)
                        bsiDPoint3d_normalizeInPlace (pBuf + i);
                    }
                status = SUCCESS;
                }
            break;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @description Transform the given array of doubles according to the transform type code.
*
* @param pBuf           IN OUT  array to transform
* @param count          IN      # doubles in array
* @param pTransform     IN      transform
* @param transformType  IN      indicates how structs are transformed
* @return SUCCESS if the transform was successful, or if it was not required
* @group    "Matrix Elements"
* @see mdlMatrixDoubleDataElement_transform
* @bsimethod                                    DavidAssaf      12/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MatrixHeaderUtils::TransformBuffer
(
        double      *pBuf,
        int         count,
const   Transform   *pTransform,
        int         transformType
)
    {
    int dim, coordType;

    if (SUCCESS != DecodeTransformType (transformType, &coordType, &dim, NULL))
        return ERROR;

    /* no transform necessary */
    if (coordType == MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_NONE)
        return SUCCESS;

    switch (dim)
        {
        case 1:
            return TransformScalarBuffer (pBuf, count, pTransform, transformType);
            break;

        case 2:
            return TransformDPoint2dBuffer ((DPoint2d *) pBuf, count / 2, pTransform, transformType);
            break;

        case 3:
            return TransformDPoint3dBuffer ((DPoint3d *) pBuf, count / 3, pTransform, transformType);
            break;

        case 4:
            /* NOOP: not supported */
            break;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description Validate the index type (aka transform type) for a matrix int data element.
*
* @param transformType  IN      code to validate
* @return SUCCESS if valid transform type code
* @group    "Matrix Elements"
* @bsimethod                                    DavidAssaf      12/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MatrixHeaderUtils::ValidateIntIndexType
(
int transformType
)
    {
    int block = transformType & MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_MASK;
    int base = transformType & MATRIX_DATA_ELM_INDEX_TYPE_BASE_MASK;
    int pad = transformType & MATRIX_DATA_ELM_INDEX_TYPE_PAD_MASK;
    int bSigned = transformType & MATRIX_DATA_ELM_INDEX_TYPE_SIGNED_MASK;

    /* check possible values: use #defines in mselems.h and elements.h */
    switch (block)
        {
        /* not an index array */
        case MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_NONE:
            return SUCCESS;
            break;

        case MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_FIXED:
        case MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_VARIABLE:
            break;

        default:
            return ERROR;
            break;
        }

    switch (base)
        {
        case MATRIX_DATA_ELM_INDEX_TYPE_BASE_ZERO:
        case MATRIX_DATA_ELM_INDEX_TYPE_BASE_ONE:
            break;

        default:
            return ERROR;
            break;
        }

    switch (pad)
        {
        case MATRIX_DATA_ELM_INDEX_TYPE_PAD_NONE:
        case MATRIX_DATA_ELM_INDEX_TYPE_PAD_ZERO:
        case MATRIX_DATA_ELM_INDEX_TYPE_PAD_MINUSONE:
            break;

        default:
            return ERROR;
            break;
        }

    /* rule out invalid combinations */
    if (
            (
                bSigned
                &&
                base == MATRIX_DATA_ELM_INDEX_TYPE_BASE_ZERO
            )
            ||
            (
                bSigned
                &&
                pad == MATRIX_DATA_ELM_INDEX_TYPE_PAD_MINUSONE

            )
            ||
            (
                base == MATRIX_DATA_ELM_INDEX_TYPE_BASE_ZERO
                &&
                pad == MATRIX_DATA_ELM_INDEX_TYPE_PAD_ZERO
            )
            ||
            (
                pad == MATRIX_DATA_ELM_INDEX_TYPE_PAD_NONE
                &&
                block == MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_VARIABLE
                /* fixed block with no pad means every face size = block size */
            )
        )
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description Construct the index type (aka transform type) for a matrix int data
*       element.  The pad/terminator of a fixed/variable blocked matrix is automatically
*       set according to the given base: 0 base => -1 pad; 1 base => 0 pad.
* <P>
* Pass MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_NONE for blocking if ints should not be
*       interpreted as indices (in this case, the other inputs are ignored).
* @param pTransformType OUT     code to encipher
* @param blocking       IN      MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_*
* @param base           IN      0 or 1 for 0-based or 1-based indices
* @param bPadded        IN      true to set block pad/terminator according to base; false for no padding
* @param bSigned        IN      true if indices can be signed
* @return SUCESS if valid transform type code
* @group    "Matrix Elements"
* @bsimethod                                    DavidAssaf      12/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MatrixHeaderUtils::EncodeIndexType
(
int         *pTransformType,
int         blocking,
int         base,
bool        bPadded,
bool        bSigned
)
    {
    if (!pTransformType)
        return ERROR;

    *pTransformType = (blocking & MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_MASK);

    if (blocking != MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_NONE)
        {
        switch (base)
            {
            case 0:
                *pTransformType |= MATRIX_DATA_ELM_INDEX_TYPE_BASE_ZERO;
                break;

            case 1:
                *pTransformType |= MATRIX_DATA_ELM_INDEX_TYPE_BASE_ONE;
                break;
            }

        if (bPadded)
            {
            switch (base)
                {
                case 0:
                    *pTransformType |= MATRIX_DATA_ELM_INDEX_TYPE_PAD_MINUSONE;
                    break;

                case 1:
                    *pTransformType |= MATRIX_DATA_ELM_INDEX_TYPE_PAD_ZERO;
                    break;
                }
            }
        else
            *pTransformType |= MATRIX_DATA_ELM_INDEX_TYPE_PAD_NONE;

        if (bSigned)
            *pTransformType |= MATRIX_DATA_ELM_INDEX_TYPE_SIGNED_MASK;
        }

    return ValidateIntIndexType (*pTransformType);
    }

/*---------------------------------------------------------------------------------**//**
* @description Return the information encoded in the index type (aka transform type)
*       of a matrix int data element.  NULL may be supplied for any combination of
*       output parameters.
* <P>
* If pBlocking returns (or would return, if it were not NULL) with the value
*       MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_NONE, then the other outputs are unfilled.
*       This indicates the matrix data are not to be interpreted as indices.
* @param transformType  IN      code to decipher
* @param pBlocking      OUT     MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_*
* @param pBase          OUT     (0 or 1) n-based data
* @param pbPadded       OUT     true if blocks are padded/terminated
* @param pPad           OUT     (-1 or 0) pad/terminator for fixed/variable blocks; returned only if pbPadded is true
* @param pbSigned       OUT     true if indices can be signed
* @return SUCCESS if valid transform type code
* @group    "Matrix Elements"
* @bsimethod                                    DavidAssaf      12/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MatrixHeaderUtils::DecodeIndexType
(
int         transformType,
int         *pBlocking,
int         *pBase,
bool        *pbPadded,
int         *pPad,
bool        *pbSigned
)
    {
    int blocking = transformType & MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_MASK;

    if (SUCCESS != ValidateIntIndexType (transformType))
        return ERROR;

    if (pBlocking)
        *pBlocking = blocking;

    /* code signifies "do not interpret these ints as indices" */
    if (blocking == MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_NONE)
        return SUCCESS;

    if (pBase)
        {
        switch (transformType & MATRIX_DATA_ELM_INDEX_TYPE_BASE_MASK)
            {
            case MATRIX_DATA_ELM_INDEX_TYPE_BASE_ZERO:
                *pBase = 0;
                break;

            case MATRIX_DATA_ELM_INDEX_TYPE_BASE_ONE:
                *pBase = 1;
                break;
            }
        }

    if (pbPadded)
        *pbPadded = (MATRIX_DATA_ELM_INDEX_TYPE_PAD_NONE
                        != (transformType & MATRIX_DATA_ELM_INDEX_TYPE_PAD_MASK));

    if (pPad)
        {
        switch (transformType & MATRIX_DATA_ELM_INDEX_TYPE_PAD_MASK)
            {
            case MATRIX_DATA_ELM_INDEX_TYPE_PAD_ZERO:
                *pPad = 0;
                break;

            case MATRIX_DATA_ELM_INDEX_TYPE_PAD_MINUSONE:
                *pPad = -1;
                break;
            }
        }

    if (pbSigned)
        {
        *pbSigned = (transformType & MATRIX_DATA_ELM_INDEX_TYPE_SIGNED_MASK)
                    ? true : false;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description Transform the doubles in this data element according to the transform type.
*
* @remarks The dimension of the structs in this matrix data element is encoded in its
*       transform type.  Dimension affects the transformation of a struct by the transform
*       <I>T</I> = [<I>M t</I>] as follows:
* <UL>
* <LI>1D: the <I>y/z</I>-rows/columns of M and the <I>y/z</I>-coordinates of <I>t</I> are irrelevant.  Note
*              that this means that we scale by the value of <I>M</I><SUB>0,0</SUB>, <EM>not</EM> by the
*              magnitude of the first column of <I>M</I>.
* <LI>2D: the <I>z</I>-row/column of <I>M</I> and the <I>z</I>-coordinate of <I>t</I> are irrelevant.
* <LI>3D: all of <I>T</I> is relevant.
* <LI>4D: data are not transformed by this function.
* </UL>
* @remarks The coordinate type of the structs in this matrix data element is encoded in
*       its transform type.  Coordinate type affects the transformation of a struct <I>p</I> by the
*       transform <I>T</I> = [<I>M t</I>] as follows:
* <UL>
* <LI>Point: [<I>M t</I>] <I>p</I> = <I>M p</I> + <I>t</I>
* <LI>Vector: [<I>M t</I>] <I>p</I> = <I>M p</I>
* <LI>Covector: [<I>M t</I>] <I>p</I> = <I>M</I><SUP>-^</SUP> <I>p</I>, e.g., multiplication by the
*               inverse transpose (this preserves orthogonality of the covectors)
* </UL>
* @remarks The normalization of the structs in this matrix data element is encoded in its
*       transform type.  Transformation of a normalized struct proceeds as above, and ends with
*       re-normalization to ensure that each struct has unit length.
* @param pMatrixDoubleDataElement   IN OUT  element to transform
* @param pTransform                 IN      transform
* @return SUCCESS if the transform was successful, or if it was not required
* @group    "Matrix Elements"
* @see mdlMatrixDoubleData_decodeTransformType
* @bsimethod                                    DavidAssaf      10/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt MatrixHeaderUtils::TransformDoubleDataElementInPlace
(
        DgnElement  *pMatrixDoubleDataElement,
const   Transform       *pTransform
)
    {
    int             type, count;
    double          *pBuf;

    if (pMatrixDoubleDataElement->GetLegacyType() != MATRIX_DOUBLE_DATA_ELM)
        return ERROR;

    type = pMatrixDoubleDataElement->ToMatrix_double_data().transformType;
    count = pMatrixDoubleDataElement->ToMatrix_double_data().numValue;
    pBuf = pMatrixDoubleDataElement->ToMatrix_double_dataR().data;

    return MatrixHeaderUtils::TransformBuffer (pBuf, count, pTransform, type);
    }

#define PAD_BYTES 1000
#define MAX_BYTES_MATRIX_DATA_PER_ELEMENT (MAX_V8_ELEMENT_SIZE - PAD_BYTES)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/00
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MatrixHeaderUtils::MatrixDataElementSize
(
size_t structSize,
size_t maxValue
)
    {
    size_t size = sizeof (Matrix_int_data) - sizeof (int);	// subtract off the placeholder int..not Int64

    /* ASSUMPTIONS:

        1. The size of each matrix data element is the same.
        2. Each matrix data element has 8 bytes of declared array space.
        3. The "data" field is always offset back by sizeof (Int64).
    */
    if (maxValue > 0)
        size += maxValue * structSize;

    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @remarks Ignore attributes on template; this is a non-graphics element.
* Caller is responsible for blocking large arrays into multi-element sequences
* @bsimethod                                                    EarlinLutz      08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void   MatrixHeaderUtils::CreateMatrixData
(
DgnElementR      out,
ElementHandleCP    pIn,
size_t          structSize,
size_t          maxValue,
size_t          numValue,
int             transformType,
const void      *pData,
UInt16          elementType,
DgnModelR    modelRef
)
    {
    size_t  size = 0;

    if (maxValue <= 0)
        maxValue = 1;

    if (numValue > maxValue)
        numValue = maxValue;

    size = MatrixDataElementSize (structSize, maxValue);
    // RIGHT HERE ....32/64 wall in element content
    ElementUtil::SetRequiredFields (out, pIn,
                    elementType,
                    (int)size,
                    false,  // attributes
                    false,  // scan range
                    false,  // non model
                    ElementUtil::ELEMDIM_NA,  // matrix has no dimensionality
                    &modelRef
                    );
    // NOTE: See version comment in mdlElement_createMatrixHeader.

    // init the reserved fields
    memset (out.ToMatrix_int_dataR().reserved,   0, sizeof (out.ToMatrix_int_data().reserved));
    memset (&out.ToMatrix_int_dataR().reserved2, 0, sizeof (out.ToMatrix_int_data().reserved2));

    /* Address it as matrix_int_data --- same layout for double */
    out.ToMatrix_int_dataR().maxValue      = (int)maxValue;
    out.ToMatrix_int_dataR().numValue      = (int)numValue;
    out.ToMatrix_int_dataR().transformType = transformType;

    memcpy (out.ToMatrix_int_dataR().data, pData, numValue * structSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void    MatrixHeaderUtils::CreateMatrixIntData
(
DgnElementR      out,
ElementHandleCP    pIn,
size_t          maxValue,
size_t          numValue,
int             transformType,
const int       *pData,
DgnModelR    modelRef
)
    {
    CreateMatrixData (out, pIn, sizeof (int), maxValue, numValue, transformType, pData, MATRIX_INT_DATA_ELM, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void    MatrixHeaderUtils::CreateMatrixDoubleData
(
DgnElementR      out,
ElementHandleCP    templateEh,
size_t		maxValue,
size_t		numValue,
int             transformType,
const double*   pData,
DgnModelR    modelRef
)
    {
    CreateMatrixData (out, templateEh, sizeof (double), maxValue, numValue, transformType, pData, MATRIX_DOUBLE_DATA_ELM, modelRef);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrPtr MatrixHeaderUtils::CreateIntDataChain (ElementHandleCP templateEh, int const* data, size_t numStructs, UInt32 numPerStruct, UInt32 structsPerRow, UInt32 tag, UInt32 indexFamily, UInt32 indexedBy, int transformType, DgnModelR modelRef)
    {
    DgnV8ElementBlank element;

    CreateMatrixHeader (element, templateEh,
                                numPerStruct,
                                structsPerRow,
                                tag,
                                indexFamily,
                                indexedBy,
                                modelRef);

    MSElementDescrPtr header = new MSElementDescr(element,modelRef);

    size_t maxBlockSize = MAX_BYTES_MATRIX_DATA_PER_ELEMENT / sizeof(int);
    size_t i0 = 0;
    while (i0 < numStructs)
        {
        size_t currBlockSize = numStructs - i0;
        if (currBlockSize > maxBlockSize)
            currBlockSize = maxBlockSize;
        CreateMatrixIntData (element, templateEh, currBlockSize,
            currBlockSize, transformType, &data[i0], modelRef);
        header->AddComponent(*new MSElementDescr(element,modelRef));
        i0 += currBlockSize;
        }

    return header;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrPtr MatrixHeaderUtils::CreateIntDataChain (ElementHandleCP templateEh, BlockedVectorIntR data, int transformType, DgnModelR modelRef)
    {
    DgnV8ElementBlank element;

    CreateMatrixHeader (element, templateEh,
                                data.NumPerStruct (),
                                data.StructsPerRow (),
                                data.Tag (),
                                data.IndexFamily (),
                                data.IndexedBy (),
                                modelRef);

    MSElementDescrP header = new MSElementDescr(element,modelRef);

    size_t maxBlockSize = MAX_BYTES_MATRIX_DATA_PER_ELEMENT / sizeof(int);
    size_t i0 = 0;
    size_t numValue = data.size ();
    while (i0 < numValue)
        {
        size_t currBlockSize = numValue - i0;
        if (currBlockSize > maxBlockSize)
            currBlockSize = maxBlockSize;
        CreateMatrixIntData (element, templateEh, currBlockSize,
            currBlockSize, transformType, &data[i0], modelRef);
        header->AddComponent(*new MSElementDescr(element,modelRef));
        i0 += currBlockSize;
        }

    return header;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrPtr MatrixHeaderUtils::CreateIntDataChain (ElementHandleCP templateEh, BlockedVectorUInt32R data, int transformType, DgnModelR modelRef)
    {
    DgnV8ElementBlank element;

    CreateMatrixHeader (element, templateEh,
                                data.NumPerStruct (),
                                data.StructsPerRow (),
                                data.Tag (),
                                data.IndexFamily (),
                                data.IndexedBy (),
                                modelRef);

    MSElementDescrP header = new MSElementDescr(element,modelRef);

    size_t maxBlockSize = MAX_BYTES_MATRIX_DATA_PER_ELEMENT / sizeof(UInt32);
    size_t i0 = 0;
    size_t numValue = data.size ();
    while (i0 < numValue)
        {
        size_t currBlockSize = numValue - i0;
        if (currBlockSize > maxBlockSize)
            currBlockSize = maxBlockSize;
        CreateMatrixIntData (element, templateEh, currBlockSize,
            currBlockSize, transformType, (int *) &data[i0], modelRef);
        header->AddComponent(*new MSElementDescr(element,modelRef));
        i0 += currBlockSize;
        }

    return header;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrPtr MatrixHeaderUtils::CreateDoubleDataChain (ElementHandleCP templateEh, BlockedVectorDPoint3dR data, int transformType, DgnModelR modelRef)
    {
    DgnV8ElementBlank element;
    CreateMatrixHeader (element, templateEh,
                                data.NumPerStruct (),
                                data.StructsPerRow (),
                                data.Tag (),
                                data.IndexFamily (),
                                data.IndexedBy (),
                                modelRef);
 
    MSElementDescrP header = new MSElementDescr(element,modelRef);
    int numPerStruct = 3;
    size_t maxBlockSize = MAX_BYTES_MATRIX_DATA_PER_ELEMENT / sizeof(DPoint3d);
    size_t i0 = 0;
    size_t numValue = data.size ();
    while (i0 < numValue)
        {
        size_t currBlockSize = numValue - i0;
        if (currBlockSize > maxBlockSize)
            currBlockSize = maxBlockSize;
        CreateMatrixDoubleData (element, templateEh,
                        (numPerStruct * currBlockSize),
                        (numPerStruct * currBlockSize),
                        transformType, (double*)&data[i0], modelRef);
        header->AddComponent(*new MSElementDescr(element,modelRef));
        i0 += currBlockSize;
        }
    return header;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrPtr MatrixHeaderUtils::CreateDoubleDataChain (ElementHandleCP templateEh, double const *data, size_t numStructs, UInt32 numPerStruct, UInt32 structsPerRow, UInt32 tag, UInt32 indexFamily, UInt32 indexedBy, int transformType, DgnModelR modelRef)
    {
    DgnV8ElementBlank element;
    CreateMatrixHeader (element, templateEh,
                                numPerStruct,
                                structsPerRow,
                                tag,
                                indexFamily,
                                indexedBy,
                                modelRef);

    MSElementDescrP header = new MSElementDescr(element,modelRef);
    size_t maxBlockSize = MAX_BYTES_MATRIX_DATA_PER_ELEMENT / (sizeof(double) * numPerStruct);
    size_t i0 = 0;  // data index in whole structs.
    while (i0 < numStructs)
        {
        size_t currBlockSize = numStructs - i0;
        if (currBlockSize > maxBlockSize)
            currBlockSize = maxBlockSize;
        CreateMatrixDoubleData (element, templateEh,
                        numPerStruct * currBlockSize,
                        numPerStruct * currBlockSize,
                        transformType, data + i0 * numPerStruct, modelRef);
        header->AddComponent(*new MSElementDescr(element,modelRef));
        i0 += currBlockSize;
        }
    return header;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrPtr MatrixHeaderUtils::CreateDoubleDataChain (ElementHandleCP templateEh, BlockedVectorDPoint2dR data, int transformType, DgnModelR modelRef)
    {
    DgnV8ElementBlank element;
    CreateMatrixHeader (element, templateEh,
                                data.NumPerStruct (),
                                data.StructsPerRow (),
                                data.Tag (),
                                data.IndexFamily (),
                                data.IndexedBy (),
                                modelRef);
    MSElementDescrP header = new MSElementDescr(element,modelRef);
    int numPerStruct = 2;
    size_t maxBlockSize = MAX_BYTES_MATRIX_DATA_PER_ELEMENT / sizeof(DPoint2d);
    size_t i0 = 0;
    size_t numValue = data.size ();
    while (i0 < numValue)
        {
        size_t currBlockSize = numValue - i0;
        if (currBlockSize > maxBlockSize)
            currBlockSize = maxBlockSize;
        CreateMatrixDoubleData (element, templateEh,
                        numPerStruct * currBlockSize,
                        numPerStruct * currBlockSize,
                        transformType, (double*)&data[i0], modelRef);
        header->AddComponent(*new MSElementDescr(element,modelRef));
        i0 += currBlockSize;
        }

    return header;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrPtr MatrixHeaderUtils::CreateDoubleDataChain (ElementHandleCP templateEh, BlockedVectorDVec3dR data, int transformType, DgnModelR modelRef)
    {
    DgnV8ElementBlank element;
    CreateMatrixHeader (element, templateEh,
                                data.NumPerStruct (),
                                data.StructsPerRow (),
                                data.Tag (),
                                data.IndexFamily (),
                                data.IndexedBy (),
                                modelRef);
    MSElementDescrP header = new MSElementDescr(element,modelRef);
    int numPerStruct = 3;
    size_t maxBlockSize = MAX_BYTES_MATRIX_DATA_PER_ELEMENT / sizeof(DVec3d);
    size_t i0 = 0;
    size_t numValue = data.size ();
    while (i0 < numValue)
        {
        size_t currBlockSize = numValue - i0;
        if (currBlockSize > maxBlockSize)
            currBlockSize = maxBlockSize;
        CreateMatrixDoubleData (element, templateEh,
                        numPerStruct * currBlockSize,
                        numPerStruct * currBlockSize,
                        transformType, (double*)&data[i0], modelRef);
        header->AddComponent(*new MSElementDescr(element,modelRef));
        i0 += currBlockSize;
        }
    return header;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrPtr MatrixHeaderUtils::CreateDoubleDataChain (ElementHandleCP templateEh, BlockedVectorRgbFactorR data, int transformType, DgnModelR modelRef)
    {
    DgnV8ElementBlank element;
    CreateMatrixHeader (element, templateEh,
                                data.NumPerStruct (),
                                data.StructsPerRow (),
                                data.Tag (),
                                data.IndexFamily (),
                                data.IndexedBy (),
                                modelRef);
    MSElementDescrP header = new MSElementDescr(element,modelRef);
    int numPerStruct = 3;
    size_t maxBlockSize = MAX_BYTES_MATRIX_DATA_PER_ELEMENT / sizeof(RgbFactor);
    size_t i0 = 0;
    size_t numValue = data.size ();
    while (i0 < numValue)
        {
        size_t currBlockSize = numValue - i0;
        if (currBlockSize > maxBlockSize)
            currBlockSize = maxBlockSize;
        CreateMatrixDoubleData (element, templateEh,
                        numPerStruct * currBlockSize,
                        numPerStruct * currBlockSize,
                        transformType, (double*)&data[i0], modelRef);
        header->AddComponent(*new MSElementDescr(element,modelRef));
        i0 += currBlockSize;
        }

    return header;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
void     MeshHeaderUtils::CreateMeshHeader
(
DgnElementR      out,
ElementHandleCP    templateEh,
int             meshStyle,
bool            bIs3d,
DgnModelR    modelRef
)
    {
    ElementUtil::SetRequiredFields (out, templateEh,
                MESH_HEADER_ELM, sizeof (Mesh_header),
                true,   // attributes
                true,   // scan range
                false,  // non-model
                bIs3d ? ElementUtil::ELEMDIM_3d : ElementUtil::ELEMDIM_2d,
                &modelRef);
                
    out.ToMesh_headerR().meshStyle = meshStyle;
    }

/*---------------------------------------------------------------------------------**//**
* @remarks Ignore attributes on template; this is a non-graphics element.
* @bsimethod                                                    EarlinLutz      08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void     MatrixHeaderUtils::CreateMatrixHeader
(
DgnElementR      out,
ElementHandleCP    templateEh,
UInt32          numPerStruct,
UInt32          numPerRow,
UInt32          tag,
UInt32          indexFamily,
UInt32          indexedBy,
DgnModelR    modelRef
)
    {
    ElementUtil::SetRequiredFields (out, templateEh,
                MATRIX_HEADER_ELM, sizeof (Matrix_header),
                false,  // attributes
                false,  // scan range
                false,  // non-model
                ElementUtil::ELEMDIM_NA,   // Generic matrix has no dimensionality?
                &modelRef);

    /* BB 03/01, DA4 08/04
       The matrixHeader.version field determines the usage of the UInt16[35] matrixHeader.reserved array.

       Version = 0 is V8:

            * This type was declared in the beta as having a dhdr even though isGraphics was not set,
              which means that grphgrp in the old structure was really being used to hold the component count.
              The remainder of this vestigial dhdr is now comprised of the version and reserved fields.

            * Portions of the dhdr were initialized in version 0 as follows:
               - elemUtil_initScanRangeForUnion (&pElement->hdr.dhdr.range, pElement->Is3d());
               - pElement->IsSnappable() = 1;

            * The reserved structure has junk at offsets 1, 9-16, 24, 28

        To make future use of the reserved array, increment the version field and test for the new value.
    */

    // init the reserved field
    memset (out.ToMatrix_headerR().reserved, 0, sizeof (out.ToMatrix_header().reserved));

    out.ToMatrix_headerR().numPerStruct             = numPerStruct;
    out.ToMatrix_headerR().numPerRow                = numPerRow;
    out.ToMatrix_headerR().tag                      = tag;
    out.ToMatrix_headerR().indexFamily              = indexFamily;
    out.ToMatrix_headerR().indexedBy                = indexedBy;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool MatrixHeaderUtils::Read (ElementHandleCR headerHandle, bvector <int> &data)
    {
    DgnElementCP headerElement = headerHandle.GetElementCP ();
    if (headerElement->GetLegacyType() != MATRIX_HEADER_ELM)
        return false;

    for (ChildElemIter childIter (headerHandle, ExposeChildrenReason::Count); childIter.IsValid(); childIter = childIter.ToNext())
        {
        DgnElementCP childElement = childIter.GetElementCP ();
        if (childElement->GetLegacyType() == MATRIX_INT_DATA_ELM)
            {
            int numInt = childElement->ToMatrix_int_data().numValue;
            for (int i = 0; i < numInt; i++)
                data.push_back (childElement->ToMatrix_int_data().data[i]);
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool MatrixHeaderUtils::Read (ElementHandleCR headerHandle, bvector <UInt32> &data)
    {
    DgnElementCP headerElement = headerHandle.GetElementCP ();
    if (headerElement->GetLegacyType() != MATRIX_HEADER_ELM)
        return false;

    for (ChildElemIter childIter (headerHandle, ExposeChildrenReason::Count); childIter.IsValid(); childIter = childIter.ToNext())
        {
        DgnElementCP childElement = childIter.GetElementCP ();
        if (childElement->GetLegacyType() == MATRIX_INT_DATA_ELM)
            {
            int numInt = childElement->ToMatrix_int_data().numValue;
            for (int i = 0; i < numInt; i++)
                data.push_back (childElement->ToMatrix_int_data().data[i]);
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool MatrixHeaderUtils::Read (ElementHandleCR headerHandle, bvector <double> &data)
    {
    DgnElementCP headerElement = headerHandle.GetElementCP ();
    if (headerElement->GetLegacyType() != MATRIX_HEADER_ELM)
        return false;

    for (ChildElemIter childIter (headerHandle, ExposeChildrenReason::Count); childIter.IsValid(); childIter = childIter.ToNext())
        {
        DgnElementCP childElement = childIter.GetElementCP ();
        if (childElement->GetLegacyType() == MATRIX_DOUBLE_DATA_ELM)
            {
            int numInt = childElement->ToMatrix_double_data().numValue;
            for (int i = 0; i < numInt; i++)
                data.push_back (childElement->ToMatrix_double_data().data[i]);
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool MatrixHeaderUtils::Read (ElementHandleCR headerHandle, bvector <DPoint3d> &data)
    {
    DgnElementCP headerElement = headerHandle.GetElementCP ();
    DPoint3d value;
    double buffer[3];

    if (headerElement->GetLegacyType() != MATRIX_HEADER_ELM)
        return false;

    int n = 0;
    for (ChildElemIter childIter (headerHandle, ExposeChildrenReason::Count); childIter.IsValid(); childIter = childIter.ToNext())
        {
        DgnElementCP childElement = childIter.GetElementCP ();
        if (childElement->GetLegacyType() == MATRIX_DOUBLE_DATA_ELM)
            {
            int numInt = childElement->ToMatrix_double_data().numValue;
            for (int i = 0; i < numInt; i++)
                {
                buffer[n++] = childElement->ToMatrix_double_data().data[i];
                if (n == 3)
                    {
                    value.x = buffer[0];
                    value.y = buffer[1];
                    value.z = buffer[2];
                    data.push_back (value);
                    n = 0;
                    }
                }
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool MatrixHeaderUtils::Read (ElementHandleCR headerHandle, bvector <RgbFactor> &data)
    {
    DgnElementCP headerElement = headerHandle.GetElementCP ();
    RgbFactor value;
    double buffer[3];

    if (headerElement->GetLegacyType() != MATRIX_HEADER_ELM)
        return false;

    int n = 0;
    for (ChildElemIter childIter (headerHandle, ExposeChildrenReason::Count); childIter.IsValid(); childIter = childIter.ToNext())
        {
        DgnElementCP childElement = childIter.GetElementCP ();
        if (childElement->GetLegacyType() == MATRIX_DOUBLE_DATA_ELM)
            {
            int numInt = childElement->ToMatrix_double_data().numValue;
            for (int i = 0; i < numInt; i++)
                {
                buffer[n++] = childElement->ToMatrix_double_data().data[i];
                if (n == 3)
                    {
                    value.red = buffer[0];
                    value.green = buffer[1];
                    value.blue = buffer[2];
                    data.push_back (value);
                    n = 0;
                    }
                }
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool MatrixHeaderUtils::Read (ElementHandleCR headerHandle, bvector <DPoint2d> &data)
    {
    DgnElementCP headerElement = headerHandle.GetElementCP ();
    DPoint2d value;
    double buffer[2];

    if (headerElement->GetLegacyType() != MATRIX_HEADER_ELM)
        return false;

    int n = 0;
    for (ChildElemIter childIter (headerHandle, ExposeChildrenReason::Count); childIter.IsValid(); childIter = childIter.ToNext())
        {
        DgnElementCP childElement = childIter.GetElementCP ();
        if (childElement->GetLegacyType() == MATRIX_DOUBLE_DATA_ELM)
            {
            int numInt = childElement->ToMatrix_double_data().numValue;
            for (int i = 0; i < numInt; i++)
                {
                buffer[n++] = childElement->ToMatrix_double_data().data[i];
                if (n == 2)
                    {
                    value.x = buffer[0];
                    value.y = buffer[1];
                    data.push_back (value);
                    n = 0;
                    }
                }
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool MatrixHeaderUtils::Read (ElementHandleCR headerHandle, bvector <DVec3d> &data)
    {
    DgnElementCP headerElement = headerHandle.GetElementCP ();
    DVec3d value;
    double buffer[3];

    if (headerElement->GetLegacyType() != MATRIX_HEADER_ELM)
        return false;

    int n = 0;
    for (ChildElemIter childIter (headerHandle, ExposeChildrenReason::Count); childIter.IsValid(); childIter = childIter.ToNext())
        {
        DgnElementCP childElement = childIter.GetElementCP ();
        if (childElement->GetLegacyType() == MATRIX_DOUBLE_DATA_ELM)
            {
            int numInt = childElement->ToMatrix_double_data().numValue;
            for (int i = 0; i < numInt; i++)
                {
                buffer[n++] = childElement->ToMatrix_double_data().data[i];
                if (n == 3)
                    {
                    value.x = buffer[0];
                    value.y = buffer[1];
                    value.z = buffer[2];
                    data.push_back (value);
                    n = 0;
                    }
                }
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool MatrixHeaderUtils::Read
(
ElementHandleCR headerHandle,
UInt32 &numPerStruct,
UInt32 &numPerRow,
UInt32 &tag,
UInt32 &indexFamily,
UInt32 &indexedBy
)
    {
    DgnElementCP headerElement = headerHandle.GetElementCP ();
    if (headerElement->GetLegacyType() != MATRIX_HEADER_ELM)
        return false;
    numPerStruct = headerElement->ToMatrix_header().numPerStruct;
    numPerRow    = headerElement->ToMatrix_header().numPerRow;
    tag          = headerElement->ToMatrix_header().tag;
    indexFamily  = headerElement->ToMatrix_header().indexFamily;
    indexedBy    = headerElement->ToMatrix_header().indexedBy;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeshHeaderUtils::Read (ElementHandleCR headerHandle, UInt32 &meshStyle)
    {
    DgnElementCP headerElement = headerHandle.GetElementCP ();
    if (headerElement->GetLegacyType() != MESH_HEADER_ELM)
        return false;
    meshStyle = headerElement->ToMesh_header().meshStyle;

    
    return true;
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE
