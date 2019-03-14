// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <msgeomstructs.h>
#include <Geom/bezeval.fdf>
#include <printfuncs.h>
#include <stdlib.h>

class StandardBasisPolynomial
{
private:
    double a[100];
    int degree;
public:
    StandardBasisPolynomial()
        {
        degree = -1;
        }
    StandardBasisPolynomial(double c)
        {
        degree = 0;
        a[0] = c;
        }
    void MultiplyMonomial (double c, int p)
        {
        int oldDegree = degree;
        if (p <= 0)
            return;
        degree += p;
        for (int i = 0; i <= oldDegree; i++)
            a[degree - i] = c * a[oldDegree] - i;
        for (int i = 0; i < p; i++)
            a[i] = 0.0;
        }

    // Mulitply by a linear term b + bx * x
    void MultiplyLinear (double b, double bx)
        {
        int oldDegree = degree;
        int newDegree = degree + 1;
        if (oldDegree < 0)
            return;
        a[newDegree] = a[oldDegree] * bx;
        for (int i = oldDegree; i > 0; i--)
            a[i] = b * a[i] + bx * a[i-1];
        a[0] = b * a[0];
        }

    double Eval (double x)
        {
        double s = 0.0;
        for (int i = degree; i >= 0; i--)
            s = x * s + a[i];
        return s;
        }

    double EvalIntegral (double x)
        {
        double s = 0.0;
        for (int i = degree; i >= 0; i--)
            s = x * s + a[i] / (double)(i+1);
        return s * x;
        }

    double EvalDerivative (double x, int numDerivative = 1)
        {
        double s = 0.0;
        for (int i = degree; i >= numDerivative; i--)
            {
            double b = a[i];
            for (int k = 0; k < numDerivative; k++)
                b *= (i - k);
            s = x * s + b;
            }
        return s;
        }

    int Degree () {return degree;}

    void Differentiate ()
        {
        if (degree > 0)
            {
            for (int i = 0; i < degree; i++)
                a[i] = (i+1) * a[i+1];
            a[degree] = 0.0;
            degree--;
            }
        else
            degree = -1;
        }
};




int testHermiteBezier
(
double x0,
double x1
)
    {
    double f0[10], f1[10];
    double absTol = 1.0e-14;
    double relTol = 1.0e-15;
    char message[1024];
    printDouble ("x0", x0);
    printDouble ("x1", x1);
    for (int p = 0; p < 14; p++)
        {
        StandardBasisPolynomial f(1.0);
        f.MultiplyMonomial (1.0, p);
        double fSum = 0.0;
        for (int k = 0; k <= 7;k++)
            {
            f0[k] = f.EvalDerivative (x0, k);
            f1[k] = f.EvalDerivative (x1, k);
            fSum += fabs (f0[k]);
            fSum += fabs (f1[k]);
            }
        double If = f.EvalIntegral (x1) - f.EvalIntegral (x0);
        printInt ("Power of x", p);
        printDouble ("Exact Integral", If);
        for (int numDerivative = 0; numDerivative <= 6; numDerivative++)
            {
            double h = x1 - x0;
            double Hk = bsiQuadrature_hermiteIntegral (h, f0, f1, numDerivative);
            double e = Hk - If;
            bool bExpectExact = p < 2 * (numDerivative + 1);
            sprintf (message, "BezHermiteIntegral::ExactIntegral p=%d numDerivatives=%d", p, numDerivative);
            selectTolerances (3);
            if (bExpectExact)
                checkDouble (Hk, If, message);
            else
                {
                sprintf (message, "BezHermiteIntegral numDerivatives=%d produces truncation", numDerivative);
                printDouble (message, Hk -If);
                }
            selectTolerances (0);

            double fCoff[100];
            int orderf = bsiBezier_univariateHermiteFitPoles (fCoff, x1 - x0, f0, f1, numDerivative);
            int mMax = 20;
            double eTotal = 0.0;
            double fSum = 0.0;
            for (int m = 0; m <= mMax; m++)
                {
                double u = (double)m / (double)mMax;
                double x = x0 + u * (x1 - x0);
                double fx = f.Eval (x);
                double px;
                bsiBezier_evaluate (&px, fCoff, orderf, 1, u);
                eTotal += fabs (fx - px);
                fSum += fabs (fx);
                }
            if (orderf > p)
                {
                sprintf (message, "Summed Error vs Summed f(x) at %d points" , mMax);
                selectTolerances (3);
                checkDouble (fSum + eTotal, fSum, message);
                selectTolerances (0);
                }
            }
         }
    return 0;
    }






int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    testHermiteBezier (0.0, 1.0);
    testHermiteBezier (-0.5, 1.0);
    return getExitStatus();
    }
