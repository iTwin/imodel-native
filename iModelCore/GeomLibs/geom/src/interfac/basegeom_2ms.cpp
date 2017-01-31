/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/interfac/basegeom_2ms.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*----------------------------------------------------------------------+
|TITLE Design Rationale and Format conversions between Mdl and Omdl Matrix Libraries    |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|SECTION oldMatrixProblems Design problems in the mdl transform library |
|                                                                       |
| The omdl transform library addresses the following problems in        |
| the base MS 5.5/ MS95 transform library.                              |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|DEF Data layout in transform structures
| The MS 5.5 and MS95 transform structure was organized as a 3 by 4     |
| array.  This C-style (row-major ordering) memory layout of this       |
| array causes the 3x3 matrix part of the transform and the 3-element   |
| translation part to be intermixed in memory.   The 9 doubles of the   |
| matrix part are placed in positions 0,1,2,4,5,6,8,9,10, and the       |
| 3 doubles of the vector part are placed in positions 3,7,11.          |
|                                                                       |
| Because of the intermixed layout, it was not possible to use the      |
| rotation matrix library (mdlRMatrix_...) to manipulate the rotation   |
| part of the trasnform, or the vector library (mdlVec_...) to          |
| the translation part.  Hence there were instances of both (a) simple  |
| manipulations that were available for one strucutre but not the other |
| and (b) manipulations that were made available to both by duplicate   |
| coding.                                                               |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|DEF Function names                                                     |
| Functions in the mdl transform library are named under the assumption |
| that the matrix part of the transform is actually a rotation, i.e.    |
| a 3x3 matrix whose transpose is its inverse.                          |
| Since many transforms used in practice have scaling which invalidates |
| the rotation assumption, the names are misleading.                    |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|DEF Row and column inconsistency                                       |
| At various times in the                                               |
| past, different users of the library have constructed transforms with |
| oppositely arranged rows and columns.                                 |
| That is, given the coordinates of the I,J, and K vectors of           |
| a coordinate system in space, some applications placed the vectors    |
| into the matrix as columns, others as rows.                           |
| (This problem is, strictly speaking, a problem in the use of the      |
| library, rather than in the library itself.  The new library cannot   |
| prevent users from construction transposed matrices.)                 |
| This confusion is not simply a temporary one within various mdl       |
| applications: there are instances of both row-major and column        |
| major matrices stored within the .dgn file.                           |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|DEF Environment-dependent 2D/3D switching                              |
| Every rotation and transformation function in MS 5.5 and MS95         |
| has a test of whether the current design file is 2D or 3D.  If 3D, it |
| performs the 3D transformation computations suggested by the          |
| documentation.   If 2D, the matrix is addressed, via a 2D/3D union    |
| in the typedef, as a 2D data item (i.e 2x2 matrix, 2 component        |
| translation part.  Hence an mdl program which constructs a 3D         |
| transformation by directly filling its components cannot obtain       |
| a correct 3D computation while a 2D design file is loaded.            |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
| SECTION newMatrixPolicies New policies for the omdl transform library                 |
|                                                                       |
| In order to correct these problems for the future, the omdl           |
| matrix and transform library was written with the following           |
| policies:                                                             |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|DEF Dimension of computation                                           |
| The omdl matrix and transform libraries are strictly 3D.  Users       |
| may still obtain 2D-like effects by setting appropriate z-direction   |
| entries, but the computations performed by each routine will NOT      |
| be affected by any environment settings such as the 2D/3D file switch |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|DEF Row/column confusion                                               |
| In order to encourage clear understanding of row-versus-column layout,|
| 3x3 DMatrix3d is stored as an array of 3 DPoint3d structures.         |
| structures addressable as column[i].                                  |
| Users who so desire may pass addresses of columns directly to         |
| omdlVector_... functions for manipulation as vectors.                 |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|DEF Matrix part of transform                                           |
| The matrix part of a DTransform3D transform is a DMatrix3d structure. |
| Hence any operation supported for a DMatrix3d is immediately          |
| avaiable for the matrix part of the DMatrix3d.                        |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|DEF Vector part of transform                                           |
| The vector part of a DTransform3D transform is a DPoint3d structure   |
| Hence any operation supported for a DPoint3d is immediately           |
| available for the translation part of the DMatrix3d.                  |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|DEF Function names                                                     |
| Functions names follow standard linear-algebra conventions.  Any      |
| function which are coded as if a matrix is only a rotation have this  |
| assumption clearly included in the function name.                     |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|SECTION matrixConversionFunctions Conversion functions                                         |
| Because of different design policies, mdl and omdl matrix and         |
| transform structures are physically incompatible.  That is, the       |
| compiler will not permit pointers to one type to be passed to         |
| functions operating on the other.  Therefore, explicit conversion     |
| functions are supplied.                                               |
+----------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* Copy all entries from an mdl-style transform (form3d only!!!) to DTransform3d
*
* @param        mdlTransformP => source transform
* @bsimethod                                                    EarlinLutz      07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDTransform3d_initFromTransform
(
DTransform3dP   baseGeomTransformP,
TransformCP     mdlTransformP
)
    {
    int i;
    for (i = 0; i < 3; i++)
        {
        baseGeomTransformP->matrix.column[i].x = mdlTransformP->form3d[0][i];
        baseGeomTransformP->matrix.column[i].y = mdlTransformP->form3d[1][i];
        baseGeomTransformP->matrix.column[i].z = mdlTransformP->form3d[2][i];
        }
    baseGeomTransformP->translation.x = mdlTransformP->form3d[0][3];
    baseGeomTransformP->translation.y = mdlTransformP->form3d[1][3];
    baseGeomTransformP->translation.z = mdlTransformP->form3d[2][3];
    }

/*---------------------------------------------------------------------------------**//**
* Copy all entries from an mdl-style transform (form3d only!!!) to DTransform3d,
* transposing the matrix.
*
* @param        mdlTransformP => source transform
* @bsimethod                                                    EarlinLutz      07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDTransform3d_initFromTransformTranspose
(
DTransform3dP   baseGeomTransformP,
TransformCP     mdlTransformP
)
    {
    int i;
    for (i = 0; i < 3; i++)
        {
        baseGeomTransformP->matrix.column[i].x = mdlTransformP->form3d[i][0];
        baseGeomTransformP->matrix.column[i].y = mdlTransformP->form3d[i][1];
        baseGeomTransformP->matrix.column[i].z = mdlTransformP->form3d[i][2];
        }
    baseGeomTransformP->translation.x = mdlTransformP->form3d[0][3];
    baseGeomTransformP->translation.y = mdlTransformP->form3d[1][3];
    baseGeomTransformP->translation.z = mdlTransformP->form3d[2][3];
    }


/*---------------------------------------------------------------------------------**//**
* Copy all entries from a DTransform3d to form3d in mdl-style Transform.
*
* @param        baseGeomTransformP => source transform
* @bsimethod                                                    EarlinLutz      07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiTransform_initFromDTransform3d
(
TransformP      mdlTransformP,
DTransform3dCP  baseGeomTransformP
)
    {
    int i;
    for (i = 0; i < 3; i++)
        {
        mdlTransformP->form3d[0][i] = baseGeomTransformP->matrix.column[i].x;
        mdlTransformP->form3d[1][i] = baseGeomTransformP->matrix.column[i].y;
        mdlTransformP->form3d[2][i] = baseGeomTransformP->matrix.column[i].z;
        }
    mdlTransformP->form3d[0][3] = baseGeomTransformP->translation.x;
    mdlTransformP->form3d[1][3] = baseGeomTransformP->translation.y;
    mdlTransformP->form3d[2][3] = baseGeomTransformP->translation.z;
    }

/*---------------------------------------------------------------------------------**//**
* Copy all entries from a DTransform3d to form3d in mdl-style Transform,
* transposing the matrix part.
*
* @param        baseGeomTransformP => source transform
* @bsimethod                                                    EarlinLutz      07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiTransform_initFromDTransform3dTranspose
(
TransformP      mdlTransformP,
DTransform3dCP  baseGeomTransformP
)
    {
    int i;
    for (i = 0; i < 3; i++)
        {
        mdlTransformP->form3d[i][0] = baseGeomTransformP->matrix.column[i].x;
        mdlTransformP->form3d[i][1] = baseGeomTransformP->matrix.column[i].y;
        mdlTransformP->form3d[i][2] = baseGeomTransformP->matrix.column[i].z;
        }
    mdlTransformP->form3d[0][3] = baseGeomTransformP->translation.x;
    mdlTransformP->form3d[1][3] = baseGeomTransformP->translation.y;
    mdlTransformP->form3d[2][3] = baseGeomTransformP->translation.z;
    }



/*---------------------------------------------------------------------------------**//**
* Copy all entries from an mdl-style matrix (form3d only!!!) to DMatrix3d
*
* @param        mdlMatrixP => source matrix
* @bsimethod                                                    EarlinLutz      07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDMatrix3d_initFromRotMatrix
(
DMatrix3dP  baseGeomMatrixP,
RotMatrixCP mdlMatrixP
)
    {
    int i;
    for (i = 0; i < 3; i++)
        {
        baseGeomMatrixP->column[i].x = mdlMatrixP->form3d[0][i];
        baseGeomMatrixP->column[i].y = mdlMatrixP->form3d[1][i];
        baseGeomMatrixP->column[i].z = mdlMatrixP->form3d[2][i];
        }
    }

/*---------------------------------------------------------------------------------**//**
* Copy transposed entries from an mdl-style matrix (form3d only!!!) to DMatrix3d.
*
* @param        mdlMatrixP => source matrix
* @bsimethod                                                    EarlinLutz      07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDMatrix3d_initFromRotMatrixTranspose
(
DMatrix3dP  baseGeomMatrixP,
RotMatrixCP mdlMatrixP
)
    {
    int i;
    for (i = 0; i < 3; i++)
        {
        baseGeomMatrixP->column[i].x = mdlMatrixP->form3d[i][0];
        baseGeomMatrixP->column[i].y = mdlMatrixP->form3d[i][1];
        baseGeomMatrixP->column[i].z = mdlMatrixP->form3d[i][2];
        }
    }

/*---------------------------------------------------------------------------------**//**
* Copy transposed entries from a DMatrix3d to form3d in mdl-style RotMatrix.
*
* @param        baseGeomMatrixP => source transform
* @bsimethod                                                    EarlinLutz      07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiRotMatrix_initFromDMatrix3dTranspose
(
RotMatrixP  mdlMatrixP,
DMatrix3dCP baseGeomMatrixP
)

    {
    int i;
    for (i = 0; i < 3; i++)
        {
        mdlMatrixP->form3d[i][0] = baseGeomMatrixP->column[i].x;
        mdlMatrixP->form3d[i][1] = baseGeomMatrixP->column[i].y;
        mdlMatrixP->form3d[i][2] = baseGeomMatrixP->column[i].z;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Copy all entries from a DMatrix3d to form3d in mdl-style RotMatrix.
*
* @param        baseGeomMatrixP => source transform
* @bsimethod                                                    EarlinLutz      07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiRotMatrix_initFromDMatrix3d
(
RotMatrixP  mdlMatrixP,
DMatrix3dCP baseGeomMatrixP
)
    {
    int i;
    for (i = 0; i < 3; i++)
        {
        mdlMatrixP->form3d[0][i] = baseGeomMatrixP->column[i].x;
        mdlMatrixP->form3d[1][i] = baseGeomMatrixP->column[i].y;
        mdlMatrixP->form3d[2][i] = baseGeomMatrixP->column[i].z;
        }
    }



#if defined (ROTMATRIX_FORM2D_SUPPORT)
/*---------------------------------------------------------------------------------**//**
* Copy the xy parts of a row-major 3D matrix into a row-major 2D matrix.
* @bsimethod                                                    EarlinLutz  04/98
+---------------+---------------+---------------+---------------+---------------+------*/
OldPublic void jmdlTransform_initForm2dFromForm3d
(
      Transform    *pInstance,
const Transform    *pForm3d
)
    {
    bsiTransform_initFromRowValues (pInstance,
        pForm3d->form3d[0][0],  pForm3d->form3d[0][1],  pForm3d->form3d[0][3],
        pForm3d->form3d[1][0],  pForm3d->form3d[1][1],  pForm3d->form3d[1][3],
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* Copy the a row-major 2D matrix into the xy parts of a 3D matrix. Fill
* in the z parts with 1 on diagonal, 0 elsewhere.
* @bsimethod                                                    EarlinLutz  04/98
+---------------+---------------+---------------+---------------+---------------+------*/
OldPublic void jmdlTransform_initForm3dFromForm2d
(
      Transform     *pInstance,
const Transform     *pForm2d
)
    {
    bsiTransform_initFromRowValues (pInstance,
        pForm2d->form3d[0][0],  pForm2d->form3d[0][1], 0.0, pForm2d->form3d[0][2],
        pForm2d->form3d[0][3],  pForm2d->form3d[1][0], 0.0, pForm2d->form3d[1][1],
                          0.0,                    0.0, 1.0, 0.0);
    }


/*---------------------------------------------------------------------------------**//**
* Copy the xy parts of a row-major 3D matrix into a row-major 2D matrix.
* @bsimethod                                                    EarlinLutz  04/98
+---------------+---------------+---------------+---------------+---------------+------*/
OldPublic void jmdlRotMatrix_initForm2dFromForm3d
(
      RotMatrix     *pInstance,
const RotMatrix     *pForm3d
)
    {
    bsiRotMatrix_initFromRowValues (pInstance,
        pForm3d->form3d[0][0],  pForm3d->form3d[0][1],
        pForm3d->form3d[1][0],  pForm3d->form3d[1][1],
        0.0, 0.0, 0.0, 0.0, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* Copy the a row-major 2D matrix into the xy parts of a 3D matrix. Fill
* in the z parts with 1 on diagonal, 0 elsewhere.
* @bsimethod                                                    EarlinLutz  04/98
+---------------+---------------+---------------+---------------+---------------+------*/
OldPublic void jmdlRotMatrix_initForm3dFromForm2d
(
      RotMatrix     *pInstance,
const RotMatrix     *pForm2d
)
    {
    bsiRotMatrix_initFromRowValues (pInstance,
        pForm2d->form2d[0][0],  pForm2d->form2d[0][1], 0.0,
        pForm2d->form2d[1][0],  pForm2d->form2d[1][1], 0.0,
                          0.0,                    0.0, 1.0);
    }

#endif
END_BENTLEY_GEOMETRY_NAMESPACE
