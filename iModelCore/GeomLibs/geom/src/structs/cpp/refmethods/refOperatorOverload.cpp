/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE


void operator+=(DPoint3d &point, DVec3d const &vector)
    {
        point.x = point.x + vector.x;
        point.y = point.y + vector.y;
        point.z = point.z + vector.z;
    }
void operator-=(DPoint3d &point, DVec3d const &vector)
    {
        point.x = point.x - vector.x;
        point.y = point.y - vector.y;
        point.z = point.z - vector.z;
    }
DPoint3d operator+(DPoint3d const &point, DVec3d const &vector)
    {
        DPoint3d result;
        result.x = point.x + vector.x;
        result.y = point.y + vector.y;
        result.z = point.z + vector.z;
        return result;
    }

DPoint3d operator-(DPoint3d const &point, DVec3d const &vector)
    {
        DPoint3d result;
        result.x = point.x - vector.x;
        result.y = point.y - vector.y;
        result.z = point.z - vector.z;
        return result;
    }

DVec3d operator-(DPoint3d const &point1, DPoint3d const &point0)
    {
        DVec3d result;
        result.x = point1.x - point0.x;
        result.y = point1.y - point0.y;
        result.z = point1.z - point0.z;
        return result;
    }



DPoint3d operator*( Transform const &transform, DPoint3d const &point)
    {
    DPoint3d result;
    result.x = transform.form3d[0][0] * point.x
            + transform.form3d[0][1] * point.y
            + transform.form3d[0][2] * point.z
            + transform.form3d[0][3];

    result.y = transform.form3d[1][0] * point.x
            + transform.form3d[1][1] * point.y
            + transform.form3d[1][2] * point.z
            + transform.form3d[1][3];

    result.z = transform.form3d[2][0] * point.x
            + transform.form3d[2][1] * point.y
            + transform.form3d[2][2] * point.z
            + transform.form3d[2][3];
    return result;
    }

DVec3d operator*( Transform const &transform, DVec3d const &vector)
    {
    DVec3d result;
    result.x = transform.form3d[0][0] * vector.x
            + transform.form3d[0][1] * vector.y
            + transform.form3d[0][2] * vector.z;

    result.y = transform.form3d[1][0] * vector.x
            + transform.form3d[1][1] * vector.y
            + transform.form3d[1][2] * vector.z;

    result.z = transform.form3d[2][0] * vector.x
            + transform.form3d[2][1] * vector.y
            + transform.form3d[2][2] * vector.z;
    return result;
    }


DVec3d operator*( RotMatrix const &matrix, DVec3d const &vector)
    {
    DVec3d result;
    result.x = matrix.form3d[0][0] * vector.x
            + matrix.form3d[0][1] * vector.y
            + matrix.form3d[0][2] * vector.z;

    result.y = matrix.form3d[1][0] * vector.x
            + matrix.form3d[1][1] * vector.y
            + matrix.form3d[1][2] * vector.z;

    result.z = matrix.form3d[2][0] * vector.x
            + matrix.form3d[2][1] * vector.y
            + matrix.form3d[2][2] * vector.z;
    return result;
    }



//----------------------------------------------------------------------

void operator*=(DVec3d &vector, double const scalar)
    {
        vector.x = vector.x * scalar;
        vector.y = vector.y * scalar;
        vector.z = vector.z * scalar;
    }

void operator+=(DVec3d &first, DVec3d const &second)
    {
        first.x = first.x + second.x;
        first.y = first.y + second.y;
        first.z = first.z + second.z;

    }

void operator-=(DVec3d &first, DVec3d const &second)
    {
        first.x = first.x - second.x;
        first.y = first.y - second.y;
        first.z = first.z - second.z;

    }
DVec3d operator-(DVec3d const &first, DVec3d const &second)
    {
        DVec3d result;
        result.x = first.x - second.x;
        result.y = first.y - second.y;
        result.z = first.z - second.z;
        return result;
    }

DVec3d operator+(DVec3d const &first, DVec3d const &second)
    {
        DVec3d result;
        result.x = first.x + second.x;
        result.y = first.y + second.y;
        result.z = first.z + second.z;
        return result;
    }


ValidatedDVec3d operator/(DVec3d const &vector, double const scalar)
    {
        DVec3d result;
        result.x = vector.x;
        result.y = vector.y;
        result.z = vector.z;
        bool stat = result.SafeDivide(result,scalar);
        return ValidatedDVec3d (result, stat);
    }

DVec3d operator*(double const scalar, DVec3d const &vector)
    {
        DVec3d result = vector;
        result.x = result.x * scalar;
        result.y = result.y * scalar;
        result.z = result.z * scalar;
        return result;
    }

DVec3d operator*(DVec3d const &vector, double const scalar)
    {
    DVec3d result = vector;
    result.x = result.x * scalar;
    result.y = result.y * scalar;
    result.z = result.z * scalar;
    return result;
    }

DVec3d operator*(DVec3d const &vector, Transform const &transform)
    {
    DVec3d result;
    result.x = vector.x * transform.form3d[0][0]
             + vector.y * transform.form3d[1][0]
             + vector.z * transform.form3d[2][0];
    result.y = vector.x * transform.form3d[0][1]
             + vector.y * transform.form3d[1][1]
             + vector.z * transform.form3d[2][1];
    result.z = vector.x * transform.form3d[0][2]
             + vector.y * transform.form3d[1][2]
             + vector.z * transform.form3d[2][2];
    return result;
    }

DVec3d operator*(DVec3d const &vector, RotMatrix const &matrix)
    {
    DVec3d result;
    result.x = vector.x * matrix.form3d[0][0]
        + vector.y * matrix.form3d[1][0]
        + vector.z * matrix.form3d[2][0];
    result.y = vector.x * matrix.form3d[0][1]
        + vector.y * matrix.form3d[1][1]
        + vector.z * matrix.form3d[2][1];
    result.z = vector.x * matrix.form3d[0][2]
        + vector.y * matrix.form3d[1][2]
        + vector.z * matrix.form3d[2][2];
    return result;
    }

//-----------------------------------------------------------------
Transform operator*(Transform const &transformA, Transform const &transformB)
    {
    Transform AB;
    int j;
    AB.form3d[0][3] = transformA.form3d[0][3]
                     + transformA.form3d[0][0] * transformB.form3d[0][3]
                     + transformA.form3d[0][1] * transformB.form3d[1][3]
                     + transformA.form3d[0][2] * transformB.form3d[2][3];

    AB.form3d[1][3] = transformA.form3d[1][3]
                     + transformA.form3d[1][0] * transformB.form3d[0][3]
                     + transformA.form3d[1][1] * transformB.form3d[1][3]
                     + transformA.form3d[1][2] * transformB.form3d[2][3];

    AB.form3d[2][3] = transformA.form3d[2][3]
                     + transformA.form3d[2][0] * transformB.form3d[0][3]
                     + transformA.form3d[2][1] * transformB.form3d[1][3]
                     + transformA.form3d[2][2] * transformB.form3d[2][3];

    for (j = 0; j < 3; j++)
        {

        AB.form3d[0][j]
                       = transformA.form3d[0][0] * transformB.form3d[0][j]
                       + transformA.form3d[0][1] * transformB.form3d[1][j]
                       + transformA.form3d[0][2] * transformB.form3d[2][j];

        AB.form3d[1][j]
                       = transformA.form3d[1][0] * transformB.form3d[0][j]
                       + transformA.form3d[1][1] * transformB.form3d[1][j]
                       + transformA.form3d[1][2] * transformB.form3d[2][j];

        AB.form3d[2][j]
                       = transformA.form3d[2][0] * transformB.form3d[0][j]
                       + transformA.form3d[2][1] * transformB.form3d[1][j]
                       + transformA.form3d[2][2] * transformB.form3d[2][j];
        }
    return AB;
    }

Transform operator*(Transform const &transformA, RotMatrix const &matrixB)
    {
    Transform AB;
    int j;
    AB.form3d[0][3] = transformA.form3d[0][3];
    AB.form3d[1][3] = transformA.form3d[1][3];
    AB.form3d[2][3] = transformA.form3d[2][3];

    for (j = 0; j < 3; j++)
        {

        AB.form3d[0][j]
            = transformA.form3d[0][0] * matrixB.form3d[0][j]
            + transformA.form3d[0][1] * matrixB.form3d[1][j]
            + transformA.form3d[0][2] * matrixB.form3d[2][j];

        AB.form3d[1][j]
            = transformA.form3d[1][0] * matrixB.form3d[0][j]
            + transformA.form3d[1][1] * matrixB.form3d[1][j]
            + transformA.form3d[1][2] * matrixB.form3d[2][j];

        AB.form3d[2][j]
            = transformA.form3d[2][0] * matrixB.form3d[0][j]
            + transformA.form3d[2][1] * matrixB.form3d[1][j]
            + transformA.form3d[2][2] * matrixB.form3d[2][j];
        }
    return AB;
    }
//--------------------------------------------------------------------------------------
RotMatrix operator*(RotMatrix const &rotMatrixA, RotMatrix const &rotMatrixB)
    {
    int       j;
    RotMatrix AB;
    AB.InitIdentity ();
    for (j = 0; j < 3; j++)
        {
        AB.form3d[0][j]
                       = rotMatrixA.form3d[0][0] * rotMatrixB.form3d[0][j]
                       + rotMatrixA.form3d[0][1] * rotMatrixB.form3d[1][j]
                       + rotMatrixA.form3d[0][2] * rotMatrixB.form3d[2][j];

        AB.form3d[1][j]
                       = rotMatrixA.form3d[1][0] * rotMatrixB.form3d[0][j]
                       + rotMatrixA.form3d[1][1] * rotMatrixB.form3d[1][j]
                       + rotMatrixA.form3d[1][2] * rotMatrixB.form3d[2][j];

        AB.form3d[2][j]
                       = rotMatrixA.form3d[2][0] * rotMatrixB.form3d[0][j]
                       + rotMatrixA.form3d[2][1] * rotMatrixB.form3d[1][j]
                       + rotMatrixA.form3d[2][2] * rotMatrixB.form3d[2][j];
        }
    return AB;
    }
Transform operator*(RotMatrix const &rotMatrixA, Transform const &transformB)
    {
    Transform AB;
    int j;
    AB.form3d[0][3] =
        + rotMatrixA.form3d[0][0] * transformB.form3d[0][3]
        + rotMatrixA.form3d[0][1] * transformB.form3d[1][3]
        + rotMatrixA.form3d[0][2] * transformB.form3d[2][3];

    AB.form3d[1][3] =
        + rotMatrixA.form3d[1][0] * transformB.form3d[0][3]
        + rotMatrixA.form3d[1][1] * transformB.form3d[1][3]
        + rotMatrixA.form3d[1][2] * transformB.form3d[2][3];

    AB.form3d[2][3] =
        + rotMatrixA.form3d[2][0] * transformB.form3d[0][3]
        + rotMatrixA.form3d[2][1] * transformB.form3d[1][3]
        + rotMatrixA.form3d[2][2] * transformB.form3d[2][3];

    for (j = 0; j < 3; j++)
        {

        AB.form3d[0][j]
            = rotMatrixA.form3d[0][0] * transformB.form3d[0][j]
            + rotMatrixA.form3d[0][1] * transformB.form3d[1][j]
            + rotMatrixA.form3d[0][2] * transformB.form3d[2][j];

        AB.form3d[1][j]
            = rotMatrixA.form3d[1][0] * transformB.form3d[0][j]
            + rotMatrixA.form3d[1][1] * transformB.form3d[1][j]
            + rotMatrixA.form3d[1][2] * transformB.form3d[2][j];

        AB.form3d[2][j]
            = rotMatrixA.form3d[2][0] * transformB.form3d[0][j]
            + rotMatrixA.form3d[2][1] * transformB.form3d[1][j]
            + rotMatrixA.form3d[2][2] * transformB.form3d[2][j];
        }
    return AB;
    }
//! operator overload, field by field bitwise comparison
bool operator == (DPoint3dCR dataA, DPoint3dCR dataB)
    {
    return dataA.x == dataB.x && dataA.y == dataB.y && dataA.z == dataB.z;
    }
//! operator overload, field by field bitwise comparison
bool operator == (DPoint2dCR dataA, DPoint2dCR dataB)
    {
    return dataA.x == dataB.x && dataA.y == dataB.y;
    }

DVec2d operator* (DVec2d const &vector, double scale) { return DVec2d::From (vector.x * scale, vector.y * scale);}
DVec2d operator* (double scale, DVec2d const &vector) { return DVec2d::From (vector.x * scale, vector.y * scale);}
DPoint2d operator+ (DPoint2d const &point, DVec2d const &vector) { return DVec2d::From(point.x + vector.x, point.y + vector.y);}
DPoint2d operator- (DPoint2d const &point, DVec2d const &vector) { return DVec2d::From (point.x - vector.x, point.y - vector.y); }
DVec2d operator+ (DVec2d const &vectorA, DVec2d const &vectorB) { return DVec2d::From (vectorA.x + vectorB.x, vectorA.y + vectorB.y); }



//--------------------------------------------------------------------------------------
DMatrix4d operator*(DMatrix4d const &matrixA, DMatrix4d const &matrixB)
    {
    DMatrix4d AB;   // uninitialized -- each entry is directly assigned.
    for (uint32_t i = 0; i < 4; i++)
        for (uint32_t j = 0; j < 4; j++)
            {
            AB.coff[i][j] =
                  matrixA.coff[i][0] * matrixB.coff[0][j]
                + matrixA.coff[i][1] * matrixB.coff[1][j]
                + matrixA.coff[i][2] * matrixB.coff[2][j]
                + matrixA.coff[i][3] * matrixB.coff[3][j];
            }
    return AB;
    }

//--------------------------------------------------------------------------------------
DPoint4d operator*(DMatrix4d const &matrixA, DPoint4d const &pointB)
    {
    double CC[4];
    for (uint32_t i = 0; i < 4; i++)
        {
        CC[i] =
              matrixA.coff[i][0] * pointB.x
            + matrixA.coff[i][1] * pointB.y
            + matrixA.coff[i][2] * pointB.z
            + matrixA.coff[i][3] * pointB.w;
        }
    DPoint4d C;
    C.x = CC[0]; C.y = CC[1]; C.z = CC[2]; C.w = CC[3];
    return C;
    }


//--------------------------------------------------------------------------------------
DMatrix4d operator-(DMatrix4d const &matrixA, DMatrix4d const &matrixB)
    {
    DMatrix4d C;   // uninitialized -- each entry is directly assigned.
    for (uint32_t i = 0; i < 4; i++)
        for (uint32_t j = 0; j < 4; j++)
            {
            C.coff[i][j] = matrixA.coff[i][j] - matrixB.coff[i][j];
            }
    return C;
    }

//--------------------------------------------------------------------------------------
DMatrix4d operator+(DMatrix4d const &matrixA, DMatrix4d const &matrixB)
    {
    DMatrix4d C;   // uninitialized -- each entry is directly assigned.
    for (uint32_t i = 0; i < 4; i++)
        for (uint32_t j = 0; j < 4; j++)
            {
            C.coff[i][j] = matrixA.coff[i][j] + matrixB.coff[i][j];
            }
    return C;
    }


//==============================================================================
void operator+=(FPoint3d &point, FVec3d const &vector)
    {
        point.x = point.x + vector.x;
        point.y = point.y + vector.y;
        point.z = point.z + vector.z;
    }
void operator-=(FPoint3d &point, FVec3d const &vector)
    {
        point.x = point.x - vector.x;
        point.y = point.y - vector.y;
        point.z = point.z - vector.z;
    }
FPoint3d operator+(FPoint3d const &point, FVec3d const &vector)
    {
        FPoint3d result;
        result.x = point.x + vector.x;
        result.y = point.y + vector.y;
        result.z = point.z + vector.z;
        return result;
    }

FPoint3d operator-(FPoint3d const &point, FVec3d const &vector)
    {
        FPoint3d result;
        result.x = point.x - vector.x;
        result.y = point.y - vector.y;
        result.z = point.z - vector.z;
        return result;
    }

FVec3d operator-(FPoint3d const &point1, FPoint3d const &point0)
    {
        FVec3d result;
        result.x = point1.x - point0.x;
        result.y = point1.y - point0.y;
        result.z = point1.z - point0.z;
        return result;
    }



FPoint3d operator*( Transform const &transform, FPoint3d const &point)
    {
    DPoint3d result;
    result.x = transform.form3d[0][0] * point.x
            + transform.form3d[0][1] * point.y
            + transform.form3d[0][2] * point.z
            + transform.form3d[0][3];

    result.y = transform.form3d[1][0] * point.x
            + transform.form3d[1][1] * point.y
            + transform.form3d[1][2] * point.z
            + transform.form3d[1][3];

    result.z = transform.form3d[2][0] * point.x
            + transform.form3d[2][1] * point.y
            + transform.form3d[2][2] * point.z
            + transform.form3d[2][3];
    return FPoint3d::From (result);
    }

FVec3d operator*( Transform const &transform, FVec3d const &vector)
    {
    DVec3d result;
    result.x = transform.form3d[0][0] * vector.x
            + transform.form3d[0][1] * vector.y
            + transform.form3d[0][2] * vector.z;

    result.y = transform.form3d[1][0] * vector.x
            + transform.form3d[1][1] * vector.y
            + transform.form3d[1][2] * vector.z;

    result.z = transform.form3d[2][0] * vector.x
            + transform.form3d[2][1] * vector.y
            + transform.form3d[2][2] * vector.z;
    return FVec3d::From (result);
    }


FVec3d operator*( RotMatrix const &matrix, FVec3d const &vector)
    {
    DVec3d result;
    result.x = matrix.form3d[0][0] * vector.x
            + matrix.form3d[0][1] * vector.y
            + matrix.form3d[0][2] * vector.z;

    result.y = matrix.form3d[1][0] * vector.x
            + matrix.form3d[1][1] * vector.y
            + matrix.form3d[1][2] * vector.z;

    result.z = matrix.form3d[2][0] * vector.x
            + matrix.form3d[2][1] * vector.y
            + matrix.form3d[2][2] * vector.z;
    return FVec3d::From (result);
    }



//----------------------------------------------------------------------

void operator*=(FVec3d &vector, double const scalar)
    {
    vector.x = (float)(vector.x * scalar);
    vector.y = (float)(vector.y * scalar);
    vector.z = (float)(vector.z * scalar);
    }

void operator+=(FVec3d &first, FVec3d const &second)
    {
        first.x = first.x + second.x;
        first.y = first.y + second.y;
        first.z = first.z + second.z;

    }

void operator-=(FVec3d &first, FVec3d const &second)
    {
        first.x = first.x - second.x;
        first.y = first.y - second.y;
        first.z = first.z - second.z;

    }
FVec3d operator-(FVec3d const &first, FVec3d const &second)
    {
        FVec3d result;
        result.x = first.x - second.x;
        result.y = first.y - second.y;
        result.z = first.z - second.z;
        return result;
    }

FVec3d operator+(FVec3d const &first, FVec3d const &second)
    {
        FVec3d result;
        result.x = first.x + second.x;
        result.y = first.y + second.y;
        result.z = first.z + second.z;
        return result;
    }


ValidatedFVec3d operator/(FVec3d const &vector, double const scalar)
    {
    DVec3d result;
    result.x = vector.x;
    result.y = vector.y;
    result.z = vector.z;
    bool stat = result.SafeDivide(result,scalar);
    return ValidatedFVec3d (FVec3d::From (result), stat);
    }

FVec3d operator*(double const scalar, FVec3d const &vector)
    {
    return FVec3d::From
        (
        vector.x * scalar,
        vector.y * scalar,
        vector.z * scalar
        );
    }

FVec3d operator*(FVec3d const &vector, double const scalar)
    {
    return FVec3d::From
        (
        vector.x * scalar,
        vector.y * scalar,
        vector.z * scalar
        );
    }

FVec3d operator*(FVec3d const &vector, Transform const &transform)
    {
    return FVec3d::From
        (
          vector.x * transform.form3d[0][0]
        + vector.y * transform.form3d[1][0]
        + vector.z * transform.form3d[2][0],
          vector.x * transform.form3d[0][1]
        + vector.y * transform.form3d[1][1]
        + vector.z * transform.form3d[2][1],
          vector.x * transform.form3d[0][2]
        + vector.y * transform.form3d[1][2]
        + vector.z * transform.form3d[2][2]
        );
    }

FVec3d operator*(FVec3d const &vector, RotMatrix const &matrix)
    {
    return FVec3d::From
        (
          vector.x * matrix.form3d[0][0]
        + vector.y * matrix.form3d[1][0]
        + vector.z * matrix.form3d[2][0],
          vector.x * matrix.form3d[0][1]
        + vector.y * matrix.form3d[1][1]
        + vector.z * matrix.form3d[2][1],
          vector.x * matrix.form3d[0][2]
        + vector.y * matrix.form3d[1][2]
        + vector.z * matrix.form3d[2][2]
        );
    }



END_BENTLEY_NAMESPACE
