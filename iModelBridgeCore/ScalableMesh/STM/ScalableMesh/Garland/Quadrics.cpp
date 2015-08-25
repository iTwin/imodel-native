/*--------------------------------------------------------------------------------------+
|
|   Code taken from Michael Garland's demo application called "QSlim" (version 1.0)
|   which intends to demonstrate an algorithm of mesh simplification based on
|   Garland and Heckbert(1997) "Surface Simplification Using Quadric Error Metrics".
|   The code of the demo is said to be in the public domain.
|   See: http://www.cs.cmu.edu/afs/cs/Web/People/garland/quadrics/qslim10.html
|
|   $Revision: 1.0 $
|       $Date: 2014/09/17 $
|     $Author: Christian.Cote $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "Quadrics.h"

/*---------------------------------------------------------------------------------**//**
*   Primitive quadric construction and evaluation routines.
*
*   Construct a quadric to evaluate the squared distance of any point to the given 
*   point v. Naturally, the iso-surfaces are just spheres centered at v.  
+---------------+---------------+---------------+---------------+---------------+------*/
Mat4 Quadrix_GetVertexConstraint(const Vec3& v)
{
    Mat4 L(Mat4::identity);

    L(0, 3) = -v[0];
    L(1, 3) = -v[1];
    L(2, 3) = -v[2];
    L(3, 3) = v*v;

    L(3, 0) = L(0, 3);
    L(3, 1) = L(1, 3);
    L(3, 2) = L(2, 3);

    return L;
}

/*---------------------------------------------------------------------------------**//**
*   Construct a quadric to evaluate the squared distance of any point to the given
*   plane [ax+by+cz+d = 0].  This is the "fundamental error quadric" discussed in 
*   the paper.    
+---------------+---------------+---------------+---------------+---------------+------*/
Mat4 Quadrix_GetPlaneConstraint(double a, double b, double c, double d)
{
    Mat4 K(Mat4::zero);
    K(0, 0) = a*a;   K(0, 1) = a*b;   K(0, 2) = a*c;  K(0, 3) = a*d;
    K(1, 0) = K(0, 1); K(1, 1) = b*b;   K(1, 2) = b*c;  K(1, 3) = b*d;
    K(2, 0) = K(0, 2); K(2, 1) = K(1, 2); K(2, 2) = c*c;  K(2, 3) = c*d;
    K(3, 0) = K(0, 3); K(3, 1) = K(1, 3); K(3, 2) = K(2, 3); K(3, 3) = d*d;
    return K;
}

/*---------------------------------------------------------------------------------**//**
*   Define some other convenient ways for constructing these plane quadrics.  
+---------------+---------------+---------------+---------------+---------------+------*/
Mat4 Quadrix_GetPlaneConstraint(const Vec3& n, double d)
{
    return Quadrix_GetPlaneConstraint(n[X], n[Y], n[Z], d);
}

Mat4 Quadrix_GetPlaneConstraint(Face& T)
{
    const Plane& p = T.Plane();
    double a, b, c, d;
    p.Coeffs(&a, &b, &c, &d);
    return Quadrix_GetPlaneConstraint(a, b, c, d);
}

Mat4 Quadrix_GetPlaneConstraint(const Vec3& v1, const Vec3& v2, const Vec3& v3)
{
    Plane P(v1, v2, v3);
    double a, b, c, d;
    P.Coeffs(&a, &b, &c, &d);
    return Quadrix_GetPlaneConstraint(a, b, c, d);
}

double Quadrix_EvaluateVertex(const Vec3& v, const Mat4& K)
{
    double x = v[X], y = v[Y], z = v[Z];

    // This is the fast way of computing (v^T Q v).
    return x*x*K(0, 0) + 2 * x*y*K(0, 1) + 2 * x*z*K(0, 2) + 2 * x*K(0, 3)
        + y*y*K(1, 1) + 2 * y*z*K(1, 2) + 2 * y*K(1, 3)
        + z*z*K(2, 2) + 2 * z*K(2, 3)
        + K(3, 3);
}

/*---------------------------------------------------------------------------------**//**
*   Routines for computing discontinuity constraints.  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool is_border(Edge *e)
{
    return ClassifyEdge(e) == EDGE_BORDER;
}

bool Quadrix_CheckForDiscontinuity(Edge *e)
{
    return is_border(e);
}

Mat4 Quadrix_GetDiscontinuityConstraint(Edge *GetEdge, const Vec3& n)
{
    Vec3& org = *GetEdge->Org();
    Vec3& Dest = *GetEdge->Dest();
    Vec3 e = Dest - org;

    Vec3 n2 = e ^ n;
    unitize(n2);

    double d = -n2 * org;
    return Quadrix_GetPlaneConstraint(n2, d);
}

Mat4 Quadrix_GetDiscontinuityConstraint(Edge *edge)
{
    Mat4 D(Mat4::zero);

    face_buffer& faces = edge->FaceUses();

    for (int i = 0; i<faces.length(); i++)
        D += Quadrix_GetDiscontinuityConstraint(edge, faces(i)->Plane().Normal());

    return D;
}

/*---------------------------------------------------------------------------------**//**
*   Routines for computing contraction target. 
+---------------+---------------+---------------+---------------+---------------+------*/
bool Quadrix_FindLocalFit(const Mat4& K, const Vec3& v1, const Vec3& v2, Vec3& candidate, OptimalPlacementPolicyType policy)
{
    Vec3 v3 = (v1 + v2) / 2;

    bool try_midpoint = policy > PLACE_ENDPOINTS;

    double c1 = Quadrix_EvaluateVertex(v1, K);
    double c2 = Quadrix_EvaluateVertex(v2, K);
    double c3;
    if (try_midpoint) c3 = Quadrix_EvaluateVertex(v3, K);

    if (c1<c2)
    {
        if (try_midpoint && c3<c1)
            candidate = v3;
        else
            candidate = v1;
    }
    else
    {
        if (try_midpoint && c3<c2)
            candidate = v3;
        else
            candidate = v2;
    }
    return true;
}

bool Quadrix_FindLineFit(const Mat4& Q, const Vec3& v1, const Vec3& v2, Vec3& candidate)
{
    Vec3 d = v1 - v2;

    Vec3 Qv2 = Q*v2;
    Vec3 Qd = Q*d;

    double denom = 2 * d*Qd;

    if (denom == 0.0)
        return false;

    double a = (d*Qv2 + v2*Qd) / denom;

    if (a<0.0) a = 0.0;
    if (a>1.0) a = 1.0;

    candidate = a*d + v2;
    return true;
}

bool Quadrix_FindBestFit(const Mat4& Q, Vec3& candidate)
{
    Mat4 K = Q;
    K(3, 0) = K(3, 1) = K(3, 2) = 0.0;  K(3, 3) = 1;

    Mat4 M;
    double det = K.inverse(M);
    if (FEQ(det, 0.0, 1e-12))
    {
        return false;
    }        
    candidate[X] = M(0, 3);
    candidate[Y] = M(1, 3);
    candidate[Z] = M(2, 3);

    return true;
}

double Quadrix_PairTarget(const Mat4& Q, Vertex *v1, Vertex *v2, Vec3& candidate, OptimalPlacementPolicyType policy, bool preserveBoundaries)
{
    // This analytic boundary preservation isn't really necessary.  The
    // boundary constraint quadrics are quite effective.  But, I've left it
    // in anyway.
    if (preserveBoundaries)
    {
        int c1 = ClassifyVertex(v1);
        int c2 = ClassifyVertex(v2);

        if (c1 > c2)
        {
            candidate = *v1;
            return Quadrix_EvaluateVertex(candidate, Q);
        }
        else if (c2 > c1)
        {
            candidate = *v2;
            return Quadrix_EvaluateVertex(candidate, Q);
        }
        else if (c1 > 0 && policy > PLACE_LINE)
        {
            policy = PLACE_LINE;
        }
        //if (policy == PLACE_OPTIMAL) assert(c1 == 0 && c2 == 0);
    }
    switch (policy)
    {
        case PLACE_OPTIMAL:
            if (Quadrix_FindBestFit(Q, candidate))
                break;

        case PLACE_LINE:
            if (Quadrix_FindLineFit(Q, *v1, *v2, candidate))
                break;

        default:
            Quadrix_FindLocalFit(Q, *v1, *v2, candidate, policy);
            break;
    }
    return Quadrix_EvaluateVertex(candidate, Q);
}
