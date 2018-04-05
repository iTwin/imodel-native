/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/cpp/refmethods/refrotmatrixInvert.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE




/*-----------------------------------------------------------------*//**
* @description Return the product of a matrix inverse and a point.
* @vbexception Throws an exception if the matrix is singular (can't be inverted).
* @param [out] result the unknown point
* @param [in] point The The known point
* @return false if this instance is singular.
* @bsimethod                                                    DavidAssaf      1/99
+----------------------------------------------------------------------*/
bool RotMatrix::Solve
(
DPoint3dR result,
DPoint3dCR pointIN
) const
    {
    DVec3d  col0, col1, col2, point;
    double  det, inverseDet;
    static double s_cramerRelTol = 1.0e-10;
    this->GetColumns (col0, col1, col2);
    double a0 = col0.Magnitude ();
    double a1 = col1.Magnitude ();
    double a2 = col2.Magnitude ();
    double a = a0 + a1 + a2;
    det = col0.TripleProduct (col1, col2);
    // We would like to use cramer's rule.  But deciding when it fails is tricky.
    // Use it if the determinant is hugely clearly nonzoro.   If that fails, fall through to computing full inverse.
    // determinant is a cubed distance quantity.
    if (fabs (det) > s_cramerRelTol * (a * a * a))
        {
        inverseDet = 1.0 / det;

        point.x = col1.TripleProduct (col2, (DVec3dCR)pointIN) * inverseDet;
        point.y = col2.TripleProduct (col0, (DVec3dCR)pointIN) * inverseDet;
        point.z = col0.TripleProduct (col1, (DVec3dCR)pointIN) * inverseDet;

        result = point;
        return true;
        }

    // maybe singular.   Let the inverter make awkward decisions . . .
    RotMatrix inverse;
    result = pointIN;
    if (inverse.InverseOf (*this))
        {
        inverse.Multiply (result);
        return true;
        }
    return false;
    }


/*-----------------------------------------------------------------*//**
* @description Return the product of a matrix inverse transpose and a point.
* @vbexception Throws an exception if the matrix is singular (can't be inverted).
* @param [out] result result of the multiplication
* @param [in] point The known point multipled by the matrix inverse.
* @return false if this instance is singular.
* @bsimethod                                                    DavidAssaf      1/99
+----------------------------------------------------------------------*/
bool RotMatrix::SolveTranspose
(
DPoint3dR result,
DPoint3dCR pointIN
) const
    {
    DVec3d row0, row1, row2, point;
    double det, inverseDet;

    this->GetRows (row0, row1, row2);

    det = row0.TripleProduct (row1, row2);

    if (det != 0.0)
        {
        inverseDet = 1.0 / det;

        point.x = row1.TripleProduct (row2, (DVec3dCR)pointIN) * inverseDet;
        point.y = row2.TripleProduct (row0, (DVec3dCR)pointIN) * inverseDet;
        point.z = row0.TripleProduct (row1, (DVec3dCR)pointIN) * inverseDet;

        result = point;
        }
    else
        {
        /* Matrix is singular.  Treat it as the identity. */
        result = pointIN;
        return false;
        }

    return true;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the inverse of the a matrix.
* @vbexception Throws an exception if the matrix is singular (can't be inverted).
* @param [in] forward The input matrix
* @return true if the matrix is invertible.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool RotMatrix::InverseOf
(

RotMatrixCR forward

)
    {
    DVec3d vectorU, vectorV, vectorW;
    DVec3d vector0, vector1, vector2;
    //RotMatrix forward0 = forward;
    double det;
    double inverseDet;

    bool boolStat;
    double mag = forward.MaxAbs ();
    double scale;
#define ALWAYS_USE_CRAMERS_RULE_not
#ifdef ALWAYS_USE_CRAMERS_RULE
    static double s_relTol1 = 1.0e-18;
#else
    /* relTol1 identifies obviously safe matrices, based on
        simple determinant after scaling so largest entry is 1.
    */
    static double s_relTol1 = 1.0e-8;
    /* relTol2 identifies truly bad matrices, based on singular
        values. */
    static double s_relTol2 = 1.0e-15;
#endif

    forward.GetColumns (vectorU, vectorV, vectorW);
    if (mag == 0.0)
        {
        /* The matrix is singular.  Fill in the identity */
        this->InitIdentity ();
        return false;
        }

    scale = 1.0 / mag;
    vectorU.Scale (scale);
    vectorV.Scale (scale);
    vectorW.Scale (scale);

    det = vectorU.TripleProduct (vectorV, vectorW);

    /* Largest entry in matrix is now 1.
       Allow cramers rule if determinant is clearly nonzero
    */
    if (fabs (det) > s_relTol1)
        {
        vector0.CrossProduct (vectorV, vectorW);
        vector1.CrossProduct (vectorW, vectorU);
        vector2.CrossProduct (vectorU, vectorV);
        /* do the work of initFromColumnVectors and transpose all at once! */
        this->InitFromRowVectors (vector0, vector1, vector2);
        inverseDet = 1.0 / (mag * det);

        this->ScaleColumns (*this, inverseDet, inverseDet, inverseDet);
        boolStat = true;
        }
    else
        {
#ifdef ALWAYS_USE_CRAMERS_RULE
        this->InitIdentity ();
        boolStat = false;
#else
        /* The matrix is badly conditioned. Use singular values
            to make the hard decision */
        RotMatrix inverse;
        double condition;
        if (  bsiRotMatrix_invertRotMatrixByOrthogonalFactors (&forward, &inverse, &condition)
           && condition > s_relTol2
           )
            {
            *this = inverse;
            boolStat = true;
            }
        else
            {
            this->InitIdentity ();
            boolStat = false;
            }
#endif
        }


    return boolStat;
    }


/*-----------------------------------------------------------------*//**
* @description Inverts this instance matrix in place.
* @return true if the matrix is invertible.
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool RotMatrix::Invert
(
)
    {
    return this->InverseOf (*this);
    }






END_BENTLEY_NAMESPACE
