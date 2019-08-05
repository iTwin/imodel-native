#pragma once

#pragma region "Geometry Helpers"
#include <geom\geomapi.h>
#include <vector>

const double SmallValue = 1.0e-150;
const double Small = 1.0e-150;

double exactinit ();
double insphere (DPoint3dCR pa, DPoint3dCR pb, DPoint3dCR pc, DPoint3dCR pd, DPoint3dCR pe);
double orient3d (DPoint3dCR pa, DPoint3dCR pb, DPoint3dCR pc, DPoint3dCR pd);
double orient3dfast (DPoint3dCR pa, DPoint3dCR pb, DPoint3dCR pc, DPoint3dCR pd);

inline void GetPlaneNormal (DVec3dP pCrossProduct, DPoint3dCP pOrigin, DPoint3dCP pTarget1, DPoint3dCP pTarget2)
    {
    bsiDVec3d_crossProduct3DPoint3d (pCrossProduct, pOrigin, pTarget1, pTarget2);
    }

inline double distToSquared (const DPoint3d& p1, const DPoint3d& p2)
        {
        double dx = p1.x - p2.x;
        double dy = p1.y - p2.y;
        double dz = p1.z - p2.z;
        return (dx*dx) + (dy*dy) + (dz*dz);
        }

inline double distTo (const DPoint3d& p1, const DPoint3d& p2)
    {
    return sqrt (distToSquared (p1, p2));
    }

//inline bool IsOnPlane (DVec3dCR normal, DPoint3dCR origin, DPoint3dCR point)
//    {
//    DVec3d testVec (point.x - origin.x, point.y - origin.y, point.z - origin.z);
//    double a = normal.DotProduct (testVec);
//
//    return a == 0;
//    }
//
inline bool ToLeft (DVec3dCR normal, DPoint3dCR origin, DPoint3dCR point)
    {
    DVec3d testVec;
    testVec.x = point.x - origin.x;
    testVec.y = point.y - origin.y;
    testVec.z = point.z - origin.z;
    double a = normal.DotProduct (testVec);

    return a >= 0;
    }

#pragma endregion

struct DTetrahedron3d
    {
    public:
        DPoint3d points[4];// Points abcd, and the faces are abc bad cbd dac

    public:
        DTetrahedron3d () 
            {
            }

        DTetrahedron3d (const DTetrahedron3d& tetrahedron)
            {
            points[0] = tetrahedron.points[0];
            points[1] = tetrahedron.points[1];
            points[2] = tetrahedron.points[2];
            points[3] = tetrahedron.points[3];
            }

        DTetrahedron3d (DPoint3dCR ptA, DPoint3dCR ptB, DPoint3dCR ptC, DPoint3dCR ptD)
            {
            points[0] = ptA;
            points[1] = ptB;
            points[2] = ptC;
            points[3] = ptD;
            }

        DTetrahedron3d (DPoint3d *pointsABCD)                            // flat array input, 4 points in array
            {
            points[0] = pointsABCD[0];
            points[1] = pointsABCD[1];
            points[2] = pointsABCD[2];
            points[3] = pointsABCD[3];
            }

        DTetrahedron3d (bvector<DPoint3d> &points) // expect 4 or more points in the bvector
            {
            points[0] = points[0];
            points[1] = points[1];
            points[2] = points[2];
            points[3] = points[3];
            }

        DTetrahedron3d (bvector<DPoint3d> &points, size_t  iA, size_t iB, size_t iC, size_t iD) // construct from indices in the bvector.
            {
            points[0] = points[iA];
            points[1] = points[iB];
            points[2] = points[iC];
            points[3] = points[iD];
            }

        void Init (DPoint3dCR ptA, DPoint3dCR ptB, DPoint3dCR ptC, DPoint3dCR ptD)
            {
            points[0] = ptA;
            points[1] = ptB;
            points[2] = ptC;
            points[3] = ptD;
            }

        DPoint3d BarycentricToPoint(double wA, double wB, double wC, double wD)   //  sum of weights times corresponding points.
            {
            }

        DPoint3d BarycentricToPoint (DPoint4dCR fractions)       // xyzw weights for ABCD
            {
            return BarycentricToPoint (fractions.x, fractions.y, fractions.z, fractions.w);
            }

        DPoint3d FractionToPoint (double uAB, double uAC, double uAD)            // treat pointA as origin, others as axis targets.  Equivalent to BarycentricToPoint (1-uA-uB-uC, uA, uB, uC)
            {
            return BarycentricToPoint (1 - uAB - uAC - uAD, uAB, uAC, uAD);
            }
        /*
        Transform GetTransform (int index0 = 0)                      // return (skewed) transform with
        // origin at point[index0]
        // x column from point[index0] to point[index0+1]
        // y axis from point[index0] to point[index0+1]
        // z axis from point[index0] to point[index+2]
        // (and all indices are cyclic modulo 4)

        Transform GetTransform (int index0, int index1, int index2, int index3)                          // return transform with origin at index0, x column from index0 to index1, etc.
        */
        DPoint4d PointToBarycentric (DPoint3dCR spacePoint)                  // inverse of BarycentricToPoint
            {
            DVec3d e0 = DVec3d::FromStartEnd (points[0], points[3]);
            DVec3d e1 = DVec3d::FromStartEnd (points[1], points[3]);
            DVec3d e2 = DVec3d::FromStartEnd (points[2], points[3]);

            double detT = e0.x *e1.y *e2.z + e0.y *e1.z *e2.x
                + e0.z *e1.x *e2.y - e0.x *e1.z *e2.y
                - e0.y *e1.x *e2.z - e0.z *e1.y *e2.x;

            if (fabs (detT) < Small)
                {
                // Degenerate tetrahedron, returning 1/4 barycentric coordinates.
                return DPoint4d::From (0.25, 0.25, 0.25, 0.25);
                }



            DVec3d res;
            /*
            = DVec3d::FromCrossProduct (inv (t, detT). (DVec3d::FromStartEnd (pt - d_);


            t.yy()*t.zz() - t.zy()*t.yz(),
            t.xz()*t.zy() - t.xy()*t.zz(),
            t.xy()*t.yz() - t.xz()*t.yy(),

            t.zx()*t.yz() - t.yx()*t.zz(),
            t.xx()*t.zz() - t.xz()*t.zx(),
            t.yx()*t.xz() - t.xx()*t.yz(),

            t.yx()*t.zy() - t.yy()*t.zx(),
            t.xy()*t.zx() - t.xx()*t.zy(),
            t.xx()*t.yy() - t.yx()*t.xy()
            */

            return DPoint4d::From (res.x, res.y, res.z, (1.0 - res.x - res.y - res.z));

            }
            /*
        DPoint3d PointToFraction (DPoint3dCR spacePoint)                         // inverse of FractionToPoint
        */

        static double dot (double* v1, double* v2)
            {
            return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
            }

        // cross() computes the cross product: n = v1 cross v2.

        static void cross (double* v1, double* v2, double* n)
            {
            n[0] = v1[1] * v2[2] - v2[1] * v1[2];
            n[1] = -(v1[0] * v2[2] - v2[0] * v1[2]);
            n[2] = v1[0] * v2[1] - v2[0] * v1[1];
            }

        static bool lu_decmp (double lu[4][4], int n, int* ps, double* d, int N)
            {
            double scales[4];
            double pivot, biggest, mult, tempf;
            int pivotindex = 0;
            int i, j, k;

            *d = 1.0;                                      // No row interchanges yet.

            for (i = N; i < n + N; i++)
                {                             // For each row.
                // Find the largest element in each row for row equilibration
                biggest = 0.0;
                for (j = N; j < n + N; j++)
                    if (biggest < (tempf = fabs (lu[i][j])))
                        biggest = tempf;
                if (biggest != 0.0)
                    scales[i] = 1.0 / biggest;
                else
                    {
                    scales[i] = 0.0;
                    return false;                            // Zero row: singular matrix.
                    }
                ps[i] = i;                                 // Initialize pivot sequence.
                }

            for (k = N; k < n + N - 1; k++)
                {                      // For each column.
                // Find the largest element in each column to pivot around.
                biggest = 0.0;
                for (i = k; i < n + N; i++)
                    {
                    if (biggest < (tempf = fabs (lu[ps[i]][k]) * scales[ps[i]]))
                        {
                        biggest = tempf;
                        pivotindex = i;
                        }
                    }
                if (biggest == 0.0)
                    {
                    return false;                         // Zero column: singular matrix.
                    }
                if (pivotindex != k)
                    {                         // Update pivot sequence.
                    j = ps[k];
                    ps[k] = ps[pivotindex];
                    ps[pivotindex] = j;
                    *d = -(*d);                          // ...and change the parity of d.
                    }

                // Pivot, eliminating an extra variable  each time
                pivot = lu[ps[k]][k];
                for (i = k + 1; i < n + N; i++)
                    {
                    lu[ps[i]][k] = mult = lu[ps[i]][k] / pivot;
                    if (mult != 0.0)
                        {
                        for (j = k + 1; j < n + N; j++)
                            lu[ps[i]][j] -= mult * lu[ps[k]][j];
                        }
                    }
                }

            // (lu[ps[n + N - 1]][n + N - 1] == 0.0) ==> A is singular.
            return lu[ps[n + N - 1]][n + N - 1] != 0.0;
            }

        ///////////////////////////////////////////////////////////////////////////////
        //                                                                           //
        // lu_solve()    Solves the linear equation:  Ax = b,  after the matrix A    //
        //               has been decomposed into the lower and upper triangular     //
        //               matrices L and U, where A = LU.                             //
        //                                                                           //
        // 'lu[N..n+N-1][N..n+N-1]' is input, not as the matrix 'A' but rather as    //
        // its LU decomposition, computed by the routine 'lu_decmp'; 'ps[N..n+N-1]'  //
        // is input as the permutation vector returned by 'lu_decmp';  'b[N..n+N-1]' //
        // is input as the right-hand side vector, and returns with the solution     //
        // vector. 'lu', 'n', and 'ps' are not modified by this routine and can be   //
        // left in place for successive calls with different right-hand sides 'b'.   //
        //                                                                           //
        ///////////////////////////////////////////////////////////////////////////////

        static void lu_solve (double lu[4][4], int n, int* ps, double* b, int N)
            {
            int i, j;
            double X[4], dot;

            for (i = N; i < n + N; i++) X[i] = 0.0;

            // Vector reduction using U triangular matrix.
            for (i = N; i < n + N; i++)
                {
                dot = 0.0;
                for (j = N; j < i + N; j++)
                    dot += lu[ps[i]][j] * X[j];
                X[i] = b[ps[i]] - dot;
                }

            // Back substitution, in L triangular matrix.
            for (i = n + N - 1; i >= N; i--)
                {
                dot = 0.0;
                for (j = i + 1; j < n + N; j++)
                    dot += lu[ps[i]][j] * X[j];
                X[i] = (X[i] - dot) / lu[ps[i]][i];
                }

            for (i = N; i < n + N; i++) b[i] = X[i];
            }


        double Volume () const             // triple product of 3 edge vectors from any origin.
            {
            DVec3d v1 = DVec3d::FromStartEnd (points[1], points[0]);
            DVec3d v2 = DVec3d::FromStartEnd (points[2], points[0]);
            DVec3d v3 = DVec3d::FromStartEnd (points[3], points[0]);

            DVec3d w = DVec3d::FromCrossProduct (v1, v2);
            double v = w.DotProduct (v3);
            return fabs ((1.0 / 6.0) * v);
            }


               static bool circumsphere (double* pa, double* pb, double* pc, double* pd, double* cent, double* radius)
                {
                double A[4][4], rhs[4], D;
                int indx[4];

                // Compute the coefficient matrix A (3x3).
                A[0][0] = pb[0] - pa[0];
                A[0][1] = pb[1] - pa[1];
                A[0][2] = pb[2] - pa[2];
                A[1][0] = pc[0] - pa[0];
                A[1][1] = pc[1] - pa[1];
                A[1][2] = pc[2] - pa[2];
                if (pd != NULL)
                    {
                    A[2][0] = pd[0] - pa[0];
                    A[2][1] = pd[1] - pa[1];
                    A[2][2] = pd[2] - pa[2];
                    }
                else
                    {
                    cross (A[0], A[1], A[2]);
                    }

                // Compute the right hand side vector b (3x1).
                rhs[0] = 0.5 * dot (A[0], A[0]);
                rhs[1] = 0.5 * dot (A[1], A[1]);
                if (pd != NULL)
                    {
                    rhs[2] = 0.5 * dot (A[2], A[2]);
                    }
                else
                    {
                    rhs[2] = 0.0;
                    }

                // Solve the 3 by 3 equations use LU decomposition with partial pivoting
                //   and backward and forward substitute..
                if (!lu_decmp (A, 3, indx, &D, 0))
                    {
                    if (radius != (double *)NULL) *radius = 0.0;
                    return false;
                    }
                lu_solve (A, 3, indx, rhs, 0);
                if (cent != (double *)NULL)
                    {
                    cent[0] = pa[0] + rhs[0];
                    cent[1] = pa[1] + rhs[1];
                    cent[2] = pa[2] + rhs[2];
                    }
                if (radius != (double *)NULL)
                    {
                    *radius = sqrt (rhs[0] * rhs[0] + rhs[1] * rhs[1] + rhs[2] * rhs[2]);
                    }
                return true;
                }

            void GetCircumsphere (DPoint3dR centre, double& radius) const
                {
                circumsphere ((double*)&points[0].x, (double*)&points[1].x, (double*)&points[2].x, (double*)&points[3].x, &centre.x, &radius);
            //    DVec3d a = DVec3d::FromStartEnd (points[1], points[0]);
            //    DVec3d b = DVec3d::FromStartEnd (points[2], points[0]);
            //    DVec3d c = DVec3d::FromStartEnd (points[3], points[0]);

            //double lambda = c.MagnitudeSquared () - (a.DotProduct (c));
            //double mu = b.MagnitudeSquared () - (a.DotProduct(b));

            //DVec3d ba = DVec3d::FromCrossProduct (b, a);
            //DVec3d ca = DVec3d::FromCrossProduct (c, a);

            //DVec3d num = DVec3d::FromSumOf (ba, lambda, ca, -mu);
            //double denom = c.DotProduct (ba);

            //if (fabs (denom) < SmallValue)
            //    {
            //    // Degenerate tetrahedron, returning centre instead of circumCentre.

            //    radiusSquared = 0;
            //    return;
            //    }

            //DVec3d centerVec = DVec3d::FromSumOf (a, 0.5, num, 0.5 / denom); //0.5 * (a + num / denom);
            //centre = DPoint3d::FromSumOf (points[0], centerVec);
            //radiusSquared = centerVec.LengthSquared ();
        }

            void GetCircumCircle(const int& face, DPoint3dR centre, double& radius) const
                {
                static const int faceHelper[][3] =
                    {
                        { 2, 1, 3 },
                        { 3, 0, 2 },
                        { 1, 0, 3 },
                        { 0, 1, 2 }
                    };

                circumsphere((double*)&points[faceHelper[face][0]].x, (double*)&points[faceHelper[face][1]].x, (double*)&points[faceHelper[face][2]].x, NULL, &centre.x, &radius);
                }
#ifdef USEPREDICATES
    bool IsPointIn (const DPoint3d& p) const
        {
        if (orient3d (points[0], points[1], points[2], p) > 0)
            return false;
        if (orient3d (points[1], points[0], points[3], p) > 0)
            return false;
        if (orient3d (points[2], points[1], points[3], p) > 0)
            return false;
        if (orient3d (points[3], points[0], points[2], p) > 0)
            return false;
        return true;
        }
    bool IsPointInOrOn (const DPoint3d& p) const
        {
        if (orient3d (points[0], points[1], points[2], p) >= 0)
        return false;
        if (orient3d (points[1], points[0], points[3], p) >= 0)
            return false;
        if (orient3d (points[2], points[1], points[3], p) >= 0)
            return false;
        if (orient3d (points[3], points[0], points[2], p) >= 0)
            return false;
        return true;
        }
#ifdef OLD
    bool isLeft_ (int face, DPoint3dCR p) const
        {
        if (face == 0 && orient3d (points[0], points[1], points[2], p) >= 0)
            return false;
        if (face == 1 && orient3d (points[1], points[0], points[3], p) >= 0)
            return false;
        if (face == 2 && orient3d (points[2], points[1], points[3], p) >= 0)
            return false;
        if (face == 3 && orient3d (points[3], points[0], points[2], p) >= 0)
            return false;
        return true;
        }
        bool isLeftOrOn_ (int face, DPoint3dCR p) const
            {
            if (face == 0 && orient3d (points[0], points[1], points[2], p) > 0)
                return false;
            if (face == 1 && orient3d (points[1], points[0], points[3], p) > 0)
                return false;
            if (face == 2 && orient3d (points[2], points[1], points[3], p) > 0)
                return false;
            if (face == 3 && orient3d (points[3], points[0], points[2], p) > 0)
                return false;
            return true;
            }
        bool isOn_ (int face, DPoint3dCR p) const
            {
            if (face == 0 && orient3d (points[0], points[1], points[2], p) != 0)
                return false;
            if (face == 1 && orient3d (points[1], points[0], points[3], p) != 0)
                return false;
            if (face == 2 && orient3d (points[2], points[1], points[3], p) != 0)
                return false;
            if (face == 3 && orient3d (points[3], points[0], points[2], p) != 0)
                return false;
            return true;
            }
#endif
    bool isLeft (int face, DPoint3dCR p) const
        {
        if (face == 3 && orient3d (points[0], points[1], points[2], p) >= 0)
        return false;
        if (face == 2 && orient3d (points[1], points[0], points[3], p) >= 0)
            return false;
        if (face == 0 && orient3d (points[2], points[1], points[3], p) >= 0)
            return false;
        if (face == 1 && orient3d (points[3], points[0], points[2], p) >= 0)
            return false;
        return true;
        }
    bool isLeftOrOn (int face, DPoint3dCR p) const
        {
        if (face == 3 && orient3d (points[0], points[1], points[2], p) > 0)
        return false;
        if (face == 2 && orient3d (points[1], points[0], points[3], p) > 0)
            return false;
        if (face == 0 && orient3d (points[2], points[1], points[3], p) > 0)
            return false;
        if (face == 1 && orient3d (points[3], points[0], points[2], p) > 0)
            return false;
        return true;
        }
    bool isOn (int face, DPoint3dCR p) const
        {
        if (face == 3 && orient3d (points[0], points[1], points[2], p) != 0)
        return false;
        if (face == 2 && orient3d (points[1], points[0], points[3], p) != 0)
            return false;
        if (face == 0 && orient3d (points[2], points[1], points[3], p) != 0)
            return false;
        if (face == 1 && orient3d (points[3], points[0], points[2], p) != 0)
            return false;
        return true;
        }
#else
    bool IsPointIn(const DPoint3d& p) const
        {
        DVec3d normal;
        DVec3d ap = DVec3d::FromStartEnd (points[1], p);
        GetPlaneNormal (&normal, &points[0], &points[1], &points[2]);
        if (normal.DotProduct (ap) < 0)
            return false;
        GetPlaneNormal (&normal, &points[1], &points[0], &points[3]);
        if (normal.DotProduct (ap) < 0)
            return false;
        GetPlaneNormal (&normal, &points[2], &points[1], &points[3]);
        if (normal.DotProduct (ap) < 0)
            return false;
        ap = DVec3d::FromStartEnd (points[0], p);
        GetPlaneNormal (&normal, &points[3], &points[0], &points[2]);
        if (normal.DotProduct (ap) < 0)
            return false;
        return true;
        }
        bool isLeft (int face, DPoint3dCR p) const
            {
            DVec3d normal;
            if (face == 0)
                {
                DVec3d ap = DVec3d::FromStartEnd (points[1], p);
                GetPlaneNormal (&normal, &points[0], &points[1], &points[2]);
                if (normal.DotProduct (ap) < 0)
                    return false;
                }
            else  if (face == 1)
                {
                DVec3d ap = DVec3d::FromStartEnd (points[1], p);
                GetPlaneNormal (&normal, &points[1], &points[0], &points[3]);
                if (normal.DotProduct (ap) < 0)
                    return false;
                }
            else if (face == 2)
                {
                DVec3d ap = DVec3d::FromStartEnd (points[1], p);
                GetPlaneNormal (&normal, &points[2], &points[1], &points[3]);
                if (normal.DotProduct (ap) < 0)
                    return false;
                }
            else if (face == 3)
                {
                DVec3d ap = DVec3d::FromStartEnd (points[0], p);
                GetPlaneNormal (&normal, &points[3], &points[0], &points[2]);
                if (normal.DotProduct (ap) < 0)
                    return false;
                }
            return true;
            }
        bool isOn (int face, DPoint3dCR p) const
            {
            DVec3d normal;
            if (face == 0)
                {
                DVec3d ap = DVec3d::FromStartEnd (points[1], p);
                GetPlaneNormal (&normal, &points[0], &points[1], &points[2]);
                if (normal.DotProduct (ap) == 0)
                    return true;
                }
            else  if (face == 1)
                {
                DVec3d ap = DVec3d::FromStartEnd (points[1], p);
                GetPlaneNormal (&normal, &points[1], &points[0], &points[3]);
                if (normal.DotProduct (ap) == 0)
                    return true;
                }
            else if (face == 2)
                {
                DVec3d ap = DVec3d::FromStartEnd (points[1], p);
                GetPlaneNormal (&normal, &points[2], &points[1], &points[3]);
                if (normal.DotProduct (ap) == 0)
                    return true;
                }
            else if (face == 3)
                {
                DVec3d ap = DVec3d::FromStartEnd (points[0], p);
                GetPlaneNormal (&normal, &points[3], &points[0], &points[2]);
                if (normal.DotProduct (ap) == 0)
                    return true;
                }
            return false;
            }
        bool isLeftOrOn (int face, DPoint3dCR p) const
                {
                DVec3d normal;
                if (face == 0)
                    {
                    DVec3d ap = DVec3d::FromStartEnd (points[1], p);
                    GetPlaneNormal (&normal, &points[0], &points[1], &points[2]);
                    if (normal.DotProduct (ap) <= 0)
                        return false;
                    }
                else  if (face == 1)
                    {
                    DVec3d ap = DVec3d::FromStartEnd (points[1], p);
                    GetPlaneNormal (&normal, &points[1], &points[0], &points[3]);
                    if (normal.DotProduct (ap) <= 0)
                        return false;
                    }
                else if (face == 2)
                    {
                    DVec3d ap = DVec3d::FromStartEnd (points[1], p);
                    GetPlaneNormal (&normal, &points[2], &points[1], &points[3]);
                    if (normal.DotProduct (ap) <= 0)
                        return false;
                    }
                else if (face == 3)
                    {
                    DVec3d ap = DVec3d::FromStartEnd (points[0], p);
                    GetPlaneNormal (&normal, &points[3], &points[0], &points[2]);
                    if (normal.DotProduct (ap) <= 0)
                        return false;
                    }
                return true;
                }
#endif
    DPoint3d GetCenter ()
        {
        return DPoint3d::From (0.25 * (points[0].x + points[1].x + points[2].x + points[3].x), 0.25 * (points[0].y + points[1].y + points[2].y + points[3].y), 0.25 * (points[0].z + points[1].z + points[2].z + points[3].z));
        }
    };


#define SMALL_NUM   0.00000001 // anything that avoids division overflow
// dot product (3D) which allows vector operations in arguments
#define dot(u,v)   ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)



// intersect3D_RayTriangle(): find the 3D intersection of a ray with a triangle
//    Input:  a ray R, and a triangle T
//    Output: *I = intersection point (when it exists)
//    Return: -1 = triangle is degenerate (a segment or point)
//             0 =  disjoint (no intersect)
//             1 =  intersect in unique point I1
//             2 =  are in the same plane
inline int intersect3D_RayTriangle (DPoint3d RP0, DVec3d dir, DPoint3d TV0, DPoint3d TV1, DPoint3d TV2, DPoint3d& I)
    {
    DVec3d    u, v, n;              // triangle DVec3ds
    DVec3d    w0, w;           // ray DVec3ds
    double    r, a, b;              // params to calc ray-plane intersect

    // get triangle edge DVec3ds and plane normal
    u = DVec3d::FromStartEnd (TV0, TV1);
    v = DVec3d::FromStartEnd (TV0, TV2);
    //        n =  u * v;              // cross product
    bsiDVec3d_crossProduct (&n, &u, &v);
    //if (n == (DVec3d)0)             // triangle is degenerate
    //    return -1;                  // do not deal with this case

    /*
    double NdotRayDirection = bsiDPoint3d_dotProduct (&n, &dir);
    if (NdotRayDirection == 0)
    return 2;
    double d = bsiDPoint3d_dotProduct (&n, &TV0);
    double t = -(bsiDPoint3d_dotProduct (&n, &RP0) + d) / NdotRayDirection;
    if (t < 0)
    return 0;
    DPoint3d P;
    P.x = RP0.x + t * dir.x;
    P.x = RP0.y + t * dir.y;
    P.x = RP0.z + t * dir.z;
    I = P;
    DVec3d C;
    DVec3d edge0 (TV0, TV1);
    DVec3d VP0 (TV0, P);
    bsiDVec3d_crossProduct (&C, &edge0, &VP0);
    if (bsiDPoint3d_dotProduct (&n, &C) < 0)
    return 0;

    DVec3d edge1 (TV1, TV2);
    DVec3d VP1 (TV2, P);
    bsiDVec3d_crossProduct (&C, &edge1, &VP1);
    if (bsiDPoint3d_dotProduct (&n, &C) < 0)
    return 0;

    DVec3d edge2 (TV2, TV0);
    DVec3d VP2 (TV2, P);
    bsiDVec3d_crossProduct (&C, &edge2, &VP2);
    if (bsiDPoint3d_dotProduct (&n, &C) < 0)
    return 0;
    return 1;
    */











    //dir = R.P1 - R.P0;              // ray direction DVec3d
    w0 = DVec3d::FromStartEnd (TV0, RP0);
    a = -dot (n, w0);
    b = dot (n, dir);
    if (fabs (b) < SMALL_NUM)
        {     // ray is  parallel to triangle plane
        if (a == 0)                 // ray lies in triangle plane
            return 2;
        else return 0;              // ray disjoint from plane
        }

    // get intersect point of ray with triangle plane
    r = a / b;
    if (r < 0.0)                    // ray goes away from triangle
        return 0;                   // => no intersect
    // for a segment, also test if (r > 1.0) => no intersect

    I = DPoint3d::FromSumOf (RP0, dir, r);            // intersect point of ray and plane

    // is I inside T?
    double uu, uv, vv, wu, wv, D;
    uu = dot (u, u);
    uv = dot (u, v);
    vv = dot (v, v);
    w = DVec3d::FromStartEnd (TV0, I);
    wu = dot (w, u);
    wv = dot (w, v);
    D = uv * uv - uu * vv;

    // get and test parametric coords
    double s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0)         // I is outside T
        return 0;
    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0)  // I is outside T
        return 0;

    return 1;                       // I is in T
    }


// Go through all edges, then find the matching edge.
// If the tetrahedron index is less then ignore.
// Check if the triangles are delanuny.
// The swap if ness.
// Write code to flip tetrahedrons