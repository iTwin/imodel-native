/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/complex.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/* MAP bsiComplex_add=Geom.complexAdd ENDMAP */

/*-----------------------------------------------------------------*//**
* @description Add two complex numbers.
* @param c <= complex sum (a+b)
* @param a => complex factor
* @param b => complex factor
* @group "Complex Arithmetic"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiComplex_add

(
DPoint2d        *c,     /* <= complex value a + b */
const DPoint2d  *a,     /* => complex value a */
const DPoint2d  *b      /* => complex value b */
)

    {
    c->x = a->x + b->x;
    c->y = a->y + b->y;
    }

/* MAP bsiComplex_subtract=Geom.complexSubtract ENDMAP */

/*-----------------------------------------------------------------*//**
* @description Subtract two complex numbers.
* @param c <= complex difference (a-b)
* @param a => complex factor
* @param b => complex factor
* @group "Complex Arithmetic"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiComplex_subtract

(
DPoint2d        *c,
const DPoint2d  *a,
const DPoint2d  *b
)

    {
    c->x = a->x - b->x;
    c->y = a->y - b->y;
    }

/* MAP bsiComplex_multiply=Geom.complexMultiply ENDMAP */

/*-----------------------------------------------------------------*//**
* @description Multiply two complex numbers.
* @param c <= complex product (a*b)
* @param a => complex factor
* @param b => complex factor
* @group "Complex Arithmetic"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiComplex_multiply

(
DPoint2d        *c,
const DPoint2d  *a,
const DPoint2d  *b
)

    {
    c->x = a->x * b->x - a->y * b->y;
    c->y = a->y * b->x + a->x * b->y;
    }

/* MAP bsiComplex_divide=Geom.complexDivide ENDMAP */

/*-----------------------------------------------------------------*//**
* @description Divide two complex numbers.
* @param c <= complex value a/b
* @param a => complex numerator
* @param b => complex denominator
* @group "Complex Arithmetic"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiComplex_divide

(
DPoint2d        *c,
const DPoint2d  *a,
const DPoint2d  *b
)
    {
    *c = Complex::Divide (*a, *b);
    }

DPoint2d Complex::Divide (DPoint2dCR a, DPoint2dCR b)
    {
    double  r, den;
    DPoint2d c;
    /* User should check that denomenator is non-zero */
    if (fabs(b.x) >= fabs(b.y))
        {
        r = b.y / b.x;
        den = b.x + r * b.y;
        c.x = (a.x + r * a.y) / den;
        c.y = (a.y - r * a.x) / den;
        }
    else
        {
        r = b.x / b.y;
        den = b.y + r * b.x;
        c.x = (a.x * r + a.y) / den;
        c.y = (a.y * r - a.x) / den;
        }
    return c;
    }



/* MAP bsiComplex_sqrt=Geom.complexAbs ENDMAP */

/*-----------------------------------------------------------------*//**
* @description Compute the modulus of a complex number.
* @param z => complex number
* @return (real) magnitude of z
* @group "Complex Arithmetic"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiComplex_abs

(
const DPoint2d    *z
)
    {
    double  x, y, ans, temp;

    x = fabs(z->x);
    y = fabs(z->y);

    if (x < 1.0e-15)
        ans = y;
    else if (y < 1.0e-15)
        ans = x;
    else if (x > y)
        {
        temp = y / x;
        ans = x * sqrt(1.0 + temp * temp);
        }
    else
        {
        temp = x / y;
        ans = y * sqrt(1.0 + temp * temp);
        }

    return ans;
    }


/*-----------------------------------------------------------------*//**
* @description Compute the square root of a complex number.
* @param c <= complex square root
* @param z => complex number
* @group "Complex Arithmetic"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiComplex_sqrt

(
DPoint2d        *c,
const DPoint2d  *z
)
    {
    *c = Complex::Sqrt (*z);
    }

DPoint2d Complex::Sqrt (DPoint2dCR z)    
    {
    DPoint2d c;
    double      x, y, w, r;

    if ((z.x == 0.0) && (z.y == 0.0))
        {
        c.x = c.y = 0.0;
        }
    else
        {
        x = fabs(z.x);
        y = fabs(z.y);
        if (x >= y)
            {
            r = y / x;
            w = sqrt(x) * sqrt(0.5 * (1.0 + sqrt(1.0 + r * r)));
            }
        else
            {
            r = x / y;
            w = sqrt(y) * sqrt(0.5 * (r + sqrt(1.0 + r * r)));
            }
        if (z.x >= 0.0)
            {
            c.x = w;
            c.y = z.y / (2.0 * w);
            }
        else
            {
            c.y = (z.y >= 0) ? w : -w;
            c.x = z.y / (2.0 * c.y);
            }
        }
    return c;
    }

/* MAP bsiComplex_realMultiplyComplex=Geom.realMultiplyComplex ENDMAP */

/*-----------------------------------------------------------------*//**
* @description Scale a complex number.
* @param c <= complex product
* @param x => real factor
* @param a => complex factor a
* @group "Complex Arithmetic"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiComplex_realMultiplyComplex

(
DPoint2d        *c,
double           x,
const DPoint2d  *a
)

    {
    c->x = x * a->x;
    c->y = x * a->y;
    }

//! distance {Magnitude(A0 - A1)}
//! @param [in] A0 complex
//! @param [in] A1 complex
double Complex::Distance
(
DPoint2dCR A0,
DPoint2dCR A1
)
    {
    double dx = A0.x - A1.x;
    double dy = A0.y - A1.y;
    return sqrt (dx * dx + dy * dy);
    }

//! difference {A0 - A1}
//! @param [in] A0 complex
//! @param [in] A1 complex
DPoint2d Complex::Subtract
(
DPoint2dCR A0,
DPoint2dCR A1
)
    {
    return DPoint2d::From (A0.x - A1.x, A0.y - A1.y);
    }

//! sum {scale0 * A0 + scale1 * A1}
//! @param [in] scale0 real factor
//! @param [in] A0 complex factor
//! @param [in] scale1 real factor
//! @param [in] A1 complex factor
DPoint2d Complex::Add
(
double scale0,
DPoint2dCR A0,
double scale1,
DPoint2dCR A1
)
    {
    return DPoint2d::From (scale0 * A0.x + scale1 * A1.x,
                           scale0 * A0.y + scale1 * A1.y);
    }

//! complex magnitude
//! @param [in] A complex vector
double Complex::Magnitude
(
DPoint2dCR A
)
    {
    return sqrt (A.x * A.x + A.y * A.y);
    }

//!  { A0 + A1}
//! @param [in] A0 complex factor
//! @param [in] A1 complex factor
DPoint2d Complex::Add
(
DPoint2dCR A0,
DPoint2dCR A1
)
    {
    return DPoint2d::From (A0.x + A1.x, A0.y + A1.y);
    }

//!  { A0 + A1 + A2}
//! @param [in] A0 complex
//! @param [in] A1 complex
//! @param [in] A1 complex
DPoint2d Complex::Add
(
DPoint2dCR A0,
DPoint2dCR A1,
DPoint2dCR A2
)
    {
    return DPoint2d::From (A0.x + A1.x + A2.x, A0.y + A1.y + A2.y);
    }

//!  { A0 + A1 + A2}
//! @param [in] A0 complex
//! @param [in] A1 complex
//! @param [in] A1 complex
DPoint2d Complex::Multiply
(
DPoint2dCR A0,
DPoint2dCR A1,
DPoint2dCR A2
)
    {
    return Multiply (A0, Multiply (A1, A2));
    }
//! complex product {A * B }
//! @param [in] A complex factor
//! @param [in] B complex factor
DPoint2d Complex::Multiply
(
DPoint2dCR A,
DPoint2dCR B
)
    {
    return DPoint2d::From (
                (A.x * B.x - A.y * B.y),
                (A.x * B.y + A.y * B.x));
    }           

//! complex product {scale * A * B }
//! @param [in] scale real factor
//! @param [in] A complex factor
//! @param [in] B complex factor
DPoint2d Complex::Multiply
(
double scale,
DPoint2dCR A,
DPoint2dCR B
)
    {
    return DPoint2d::From (
                scale * (A.x * B.x - A.y * B.y),
                scale * (A.x * B.y + A.y * B.x));
    }           

//! complex product {scale * A}
//! @param [in] scale real factor
//! @param [in] A complex factor
//! @param [in] B complex factor
DPoint2d Complex::Multiply
(
double scale,
DPoint2dCR A
)
    {
    return DPoint2d::From (scale * A.x, scale * A.y);
    }           

//! sum {scale0 * A0 * B0 + scale1 * A1 * B1}
//! @param [in] scale0 real factor
//! @param [in] A0 complex factor
//! @param [in] B0 complex factor
//! @param [in] scale1 real factor
//! @param [in] A1 complex factor
//! @param [in] B1 complex factor
DPoint2d Complex::Add
(
double scale0,
DPoint2dCR A0,
DPoint2dCR B0,
double scale1,
DPoint2dCR A1,
DPoint2dCR B1
)
    {
    return Add (Multiply (scale0, A0, B0), Multiply (scale1, A1, B1));
    }

//! sum {scale0 * A0 + scale1 * A1 * B1}
//! @param [in] scale0 real factor
//! @param [in] A0 complex factor
//! @param [in] scale1 real factor
//! @param [in] A1 complex factor
//! @param [in] B1 complex factor
DPoint2d Complex::Add
(
double scale0,
DPoint2dCR A0,
double scale1,
DPoint2dCR A1,
DPoint2dCR B1
)
    {
    return Add (Multiply (scale0, A0), Multiply (scale1, A1, B1));
    }

//! Solve quadratic equation {ax^2 + bx + c = 0}, with all coefficients complex.
//! @param [in] coffA quadratic coefficient
//! @param [in] coffB linear coefficient
//! @param [in] coffC constant coefficient
//! @return false if a coefficient is zero.
bool Complex::SolveQuadratic
(
DPoint2dCR coffA,
DPoint2dCR coffB,
DPoint2dCR coffC,
DPoint2dR root0,
DPoint2dR root1
)
    {
    root0.Zero ();
    root1.Zero ();
    if (coffA.x == 0.0 && coffA.y == 0.0)
        return false;

    DPoint2d rootDisc, denominator, numerator0, numerator1;

    DPoint2d disc = Complex::Add (1.0, coffB, coffB, -4.0, coffA, coffC);
    bsiComplex_sqrt (&rootDisc, &disc);

    // root = (-b +- rootDisc) / 2a
    numerator0 = Add (-1.0, coffB, 1.0, rootDisc);
    numerator1 = Add (-1.0, coffB, -1.0, rootDisc);
    bsiComplex_realMultiplyComplex (&denominator, 2.0, &coffA);
    bsiComplex_divide (&root0, &numerator0, &denominator);
    bsiComplex_divide (&root1, &numerator1, &denominator);
    return true;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
