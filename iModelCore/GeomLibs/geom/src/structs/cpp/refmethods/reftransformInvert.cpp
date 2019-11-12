/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE


/*-----------------------------------------------------------------*//**
*
* Sets this instance to a matrix which is the inverse of in
* IN THE SPECIAL CASE WHERE in HAS ONLY PURE ROTATION OR
* MIRRORING IN ITS ROTATIONAL PART.   These special conditions allow
* the 'inversion' to be done by only a transposition and one
* matrix-times-point multiplication, rather than the full effort of
* inverting a general transformation. It is the caller's responsibility
* to be sure that these special conditions hold.  This usually occurs
* when the caller has just constructed the transform by a sequence of
* translations and rotations.
* If the caller has received the matrix from nonverified external
* sources and therefore does not know if the special conditions apply,
* the <CODE>inverseOf</CODE> method should be used instead.
* in may be the same as this instance.
* The specific computations in this special-case inversion are (1) the
* output transform's translation is the input transform's
* matrix times the negative of the input transform's translation, and (2) the
* output transform's matrix part is the tranpose of the input transform's
* matrix part.
* Symbolically, given transform [R t] return transform [R^ (R^)*(-t)]
* where ^ indicates transposition.
*
* @param [in] in The input transformation
* @see #inverseOf
* @indexVerb
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void Transform::InvertRigidBodyTransformation (TransformCR in)
    {
    DPoint3d point;
    Transform A;

    A = in;

    form3d[0][0] = A.form3d[0][0];
    form3d[1][0] = A.form3d[0][1];
    form3d[2][0] = A.form3d[0][2];

    form3d[0][1] = A.form3d[1][0];
    form3d[1][1] = A.form3d[1][1];
    form3d[2][1] = A.form3d[1][2];

    form3d[0][2] = A.form3d[2][0];
    form3d[1][2] = A.form3d[2][1];
    form3d[2][2] = A.form3d[2][2];

    A.MultiplyTransposeMatrixOnly (point,
        -A.form3d[0][3],
        -A.form3d[1][3],
        -A.form3d[2][3]
        );

    SetTranslation (point);
    }


/*-----------------------------------------------------------------*//**
* Sets this instance to the inverse transform of in.
* in may be the same as this instance.
* This is a modestly expensive floating point computation (33
* multiplies, 14 adds).
* Symbolically, given transform [R t] return transform [Q Q*(-t)]
* where Q is the inverse of matrix R.
*
* @param [in] in The input transformation
* @return true if transform is invertible
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool Transform::InverseOf (TransformCR in)
    {
    RotMatrix M;
    DPoint3d point;
    bool    boolStat;
    in.GetMatrix (M);
    boolStat = M.Invert ();
    if  (boolStat)
        {
        M.MultiplyComponents (point,
                    -in.form3d[0][3],
                    -in.form3d[1][3],
                    -in.form3d[2][3]
                    );
        InitFrom (M, point);
        }
    else
        {
        InitIdentity ();
        }
    return boolStat;
    }

ValidatedTransform Transform::ValidatedInverse () const
    {
    Transform inverse;
    bool stat = inverse.InverseOf (*this);
    return ValidatedTransform (inverse, stat);
    }


/*-----------------------------------------------------------------*//**
* Solves the linear system Tx=b, where T is this instance, b is the input
* point and x is the output point.  No simplifying assumptions are made
* regarding the matrix part of T.  Symbolically, if T = [M t], then
* x = Q (b - t), where Q is the inverse of M (i.e., the system is equivalent
* to Mx = b - t).  inPoint and outPoint may have identical addresses.
*
* @param [out] outPoint solution to system
* @param [in] inPoint The constant point of the system
* @return false if the matrix part of this instance is singular.
* @indexVerb
* @bsimethod                                                    DavidAssaf      1/99
+----------------------------------------------------------------------*/
bool Transform::Solve
(

DPoint3dR outPoint,
DPoint3dCR inPoint

) const
    {
    RotMatrix   M;
    DPoint3d    t;
    GetMatrix (M);
    GetTranslation (t);
    outPoint.DifferenceOf (inPoint, t);

    if (!M.Solve (outPoint, outPoint))
        {
        /* treat transform as if it were the identity */
        outPoint = inPoint;
        return false;
        }
    else
        return true;
    }


/*-----------------------------------------------------------------*//**
* Solves the linear systems TX=B, where T is this instance, B is the matrix
* of numPoints input points and X is the matrix of numPoints output points.
* No simplifying assumptions are made regarding the matrix part of T.
* Symbolically, if T = [M t], then for each input/output point i,
* X[i] = Q (B[i] - t), where Q is the inverse of M (i.e., the i_th system is
* equivalent to MX[i] = B[i] - t).  inPoint and outPoint may have identical
* addresses.
*
* @param [out] outPoint column points of solution matrix to system
* @param [in] inPoint The column points of constant matrix of system
* @param [in] numPoints The number of input/output points
* @return false if the matrix part of this instance is singular.
* @indexVerb
* @bsimethod                                                    DavidAssaf      1/99
+----------------------------------------------------------------------*/
bool Transform::SolveArray (
DPoint3dP outPoint,
DPoint3dCP inPoint,
      int           numPoints
) const
    {
    Transform inverse;
    bool stat = inverse.InverseOf (*this);
    if (stat)
        inverse.Multiply (outPoint, inPoint, numPoints);
    else if (outPoint != inPoint)
        {
        for (int i = 0; i < numPoints; i++)
            outPoint[i] = inPoint[i];
        }
    return stat;
    }


END_BENTLEY_NAMESPACE
