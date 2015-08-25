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

#pragma once

#include "Model.h"
#include "Mat4.h"

extern Mat4 Quadrix_GetVertexConstraint(const Vec3&);
extern Mat4 Quadrix_GetPlaneConstraint(double a, double b, double c, double d);
extern Mat4 Quadrix_GetPlaneConstraint(Face& T);
extern Mat4 Quadrix_GetPlaneConstraint(const Vec3& n, double);
extern Mat4 Quadrix_GetPlaneConstraint(const Vec3&, const Vec3&, const Vec3&);
extern double Quadrix_EvaluateVertex(const Vec3& v, const Mat4& K);

extern bool Quadrix_CheckForDiscontinuity(Edge *);
extern Mat4 Quadrix_GetDiscontinuityConstraint(Edge *, const Vec3&);
extern Mat4 Quadrix_GetDiscontinuityConstraint(Edge *);

extern bool Quadrix_FindLocalFit(const Mat4& K, const Vec3& v1, const Vec3& v2, Vec3& candidate, OptimalPlacementPolicyType policy);
extern bool Quadrix_FindLineFit(const Mat4& Q, const Vec3& v1, const Vec3& v2, Vec3& candidate);
extern bool Quadrix_FindBestFit(const Mat4& Q, Vec3& candidate);
extern double Quadrix_PairTarget(const Mat4& Q, Vertex *v1, Vertex *v2, Vec3& candidate, OptimalPlacementPolicyType policy, bool preserveBoundaries);

