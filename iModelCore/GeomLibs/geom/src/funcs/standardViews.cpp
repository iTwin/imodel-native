/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/internal2/GeomPrivateApi.h>
#include "../DeprecatedFunctions.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE



// Yeah.  This is flakey.
static RotMatrix s_standardViewMatrix[8] = {
    {{   // Standard view matrix 1 (zero based 0)
         {1.000000000000000,  0.00000000000000000,  0.00000000000000000},
         {0.000000000000000,  1.00000000000000000,  0.00000000000000000},
         {0.000000000000000,  0.00000000000000000,  1.00000000000000000}
    }},
    {{   // Standard view matrix 2 (zero based 1)
         {1.000000000000000,  0.00000000000000000,  0.00000000000000000},
         {0.000000000000000, -1.00000000000000000,  0.00000000000000000},
         {0.000000000000000,  0.00000000000000000, -1.00000000000000000}
    }},
    {{   // Standard view matrix 3 (zero based 2)
         {0.000000000000000, -1.00000000000000000,  0.00000000000000000},
         {0.000000000000000,  0.00000000000000000,  1.00000000000000000},
         {-1.000000000000000,  0.00000000000000000,  0.00000000000000000}
    }},
    {{   // Standard view matrix 4 (zero based 3)
         {0.000000000000000,  1.00000000000000000,  0.00000000000000000},
         {0.000000000000000,  0.00000000000000000,  1.00000000000000000},
         {1.000000000000000,  0.00000000000000000,  0.00000000000000000}
    }},
    {{   // Standard view matrix 5 (zero based 4)
         {1.000000000000000,  0.00000000000000000,  0.00000000000000000},
         {0.000000000000000,  0.00000000000000000,  1.00000000000000000},
         {0.000000000000000, -1.00000000000000000,  0.00000000000000000}
    }},
    {{   // Standard view matrix 6 (zero based 5)
         {-1.000000000000000,  0.00000000000000000,  0.00000000000000000},
         {0.000000000000000,  0.00000000000000000,  1.00000000000000000},
         {0.000000000000000,  1.00000000000000000,  0.00000000000000000}
    }},
    {{   // Standard view matrix 7 (zero based 6)
         {0.707106781186548, -0.70710678118654757,  0.00000000000000000},
         {0.408248290463863,  0.40824829046386302,  0.81649658092772603},
         {-0.577350269189626, -0.57735026918962573,  0.57735026918962573}
    }},
    {{   // Standard view matrix 8 (zero based 7)
         {0.707106781186548,  0.70710678118654757,  0.00000000000000000},
         {-0.408248290463863,  0.40824829046386302,  0.81649658092772603},
         {0.577350269189626, -0.57735026918962573,  0.57735026918962573}
    }}
    };


Public GEOMDLLIMPEXP bool    bsiRotMatrix_getStandardRotation
(
RotMatrixP pInstance,
int          selector
)
    {

    int index = selector - 1;
    if (index < 0 || index >= 8)
        {
        pInstance->InitIdentity ();
        return  false;
        }
    *pInstance = s_standardViewMatrix [index];
    return  true;
    }


/// <summary>Test if matrixRV is a specified single-axis rotation of matrixV</summary>
static bool    parseRotationFromMatrix

(
RotMatrixCP pMatrixRV,
int pivotRow, // Row expected to be unchanged
int viewId,     // standard id of view to test
int *pViewIdOut,
int *pAxisIdOut,
double *pRadiansOut
)
    {
    int j, mX, mY, mZ;
    double tol = bsiTrig_smallAngle ();
    double myAngle;
    RotMatrix matrixR, matrixRXY;
    RotMatrix *pMatrixV = &s_standardViewMatrix[viewId - 1];    // ASSUME .... viewId is one based and valid!!!

    // Rotation "around pivotRow" leaves the pivot row unchanged.
    // Deviation from any element of this row is an immediate out.
    // This fact makes this whole thing cheap.

    for (j = 0; j < 3; j++)
        if (fabs (pMatrixRV->form3d[pivotRow][j] - pMatrixV->form3d[pivotRow][j]) > tol)
            return false;

    matrixR.InitProductRotMatrixRotMatrixTranspose (*pMatrixRV, *pMatrixV);

    // We have an easy test for xy rotation.
    // Do diagonal swap of other cases to xy for testing ...
    mZ  = pivotRow;  // Becomes both row and column pivot.
    mX = Angle::Cyclic3dAxis (mZ + 1);
    mY = Angle::Cyclic3dAxis (mZ + 2);
    matrixRXY.InitFromRowValues(
                matrixR.form3d[mX][mX], matrixR.form3d[mX][mY], matrixR.form3d[mX][mZ],
                matrixR.form3d[mY][mX], matrixR.form3d[mY][mY], matrixR.form3d[mY][mZ],
                matrixR.form3d[mZ][mX],  matrixR.form3d[mZ][mY],  matrixR.form3d[mZ][mZ]
                );

    if (bsiRotMatrix_isXYRotation (&matrixRXY, &myAngle))
        {
        *pViewIdOut = viewId;
        *pAxisIdOut = pivotRow;
        *pRadiansOut = myAngle;
        return true;
        }
    return false;
    }


/// <summary>Test if a matrix can be factored as R*V, where R is a primary axis rotation
///         and V is a standard view matrix.</summary>
/// <returns>true if the matrix is a rotation of the specified form.</returns>
/// <param name="pMatrix">Matrix to factor</param>
/// <param name="pRotationAxis">0,1, or 2 for rotation around x,y, or z respectively. </param>
/// <param name="pRadians">The rotation angle.</param>
/// <param name="pViewIndex">The view that the rotation starts from.</param>
/// <param name="bTestX">true to look for x axis rotations</param>
/// <param name="bTestY">true to look for y axis rotations</param>
/// <param name="bTestz">true to look for z axis rotations</param>
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isRotationFromStandardView

(
RotMatrixCP pMatrix,
int *pRotationAxis,
double *pRadians,
int *pViewIndex,
bool    bTestX,
bool    bTestY,
bool    bTestZ
)
    {
#ifndef STANDARDVIEW_Top
#define     STANDARDVIEW_Top        1
#define     STANDARDVIEW_Bottom     2
#define     STANDARDVIEW_Left       3
#define     STANDARDVIEW_Right      4
#define     STANDARDVIEW_Front      5
#define     STANDARDVIEW_Back       6
#define     STANDARDVIEW_Iso        7
#define     STANDARDVIEW_RightIso   8
#endif
   // Look for Z rotations first ....
    bool    boolstat =
        (bTestZ &&
            (  parseRotationFromMatrix (pMatrix, 2, STANDARDVIEW_Top,      pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 2, STANDARDVIEW_Front,    pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 2, STANDARDVIEW_Right,    pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 2, STANDARDVIEW_Left,     pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 2, STANDARDVIEW_Back,     pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 2, STANDARDVIEW_Bottom,   pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 2, STANDARDVIEW_Iso,      pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 2, STANDARDVIEW_RightIso, pViewIndex, pRotationAxis, pRadians)
            ))
    // Then X
        || (bTestX &&
            (  parseRotationFromMatrix (pMatrix, 0, STANDARDVIEW_Top,    pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 0, STANDARDVIEW_Right,  pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 0, STANDARDVIEW_Left,   pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 0, STANDARDVIEW_Back,   pViewIndex, pRotationAxis, pRadians)
            ))
    // Then Y
        || (bTestY &&
            (  parseRotationFromMatrix (pMatrix, 1, STANDARDVIEW_Top,    pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 1, STANDARDVIEW_Front,  pViewIndex, pRotationAxis, pRadians)
            || parseRotationFromMatrix (pMatrix, 1, STANDARDVIEW_Bottom, pViewIndex, pRotationAxis, pRadians)
            ))
        ;
    return boolstat;
    }

/// <summary>Initialize a matrix of the form R*V, where R is a primary axis rotation
///         and V is a standard view matrix.</summary>
/// <returns>true if the view select is valid.</returns>
/// <param name="rotationAxis">0,1, or 2 for rotation around x,y, or z respectively. </param>
/// <param name="radians">The rotation angle.</param>
/// <param name="viewIndex">The view that the rotation starts from.</param>
Public GEOMDLLIMPEXP bool    bsiRotMatrix_initRotationFromStandardView

(
RotMatrixP pMatrix,
int rotationAxis,
double radians,
int viewIndex
)
    {
    RotMatrix matrixR, matrixV;
    if (bsiRotMatrix_getStandardRotation (&matrixV, viewIndex))
        {
        matrixR.InitFromAxisAndRotationAngle (rotationAxis, radians);
        pMatrix->InitProduct (matrixR, matrixV);
        return true;
        }
    pMatrix->InitIdentity ();
    return false;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
