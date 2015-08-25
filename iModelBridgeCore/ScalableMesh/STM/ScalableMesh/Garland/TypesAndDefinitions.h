/*--------------------------------------------------------------------------------------+
|
|   Code partially derived from Michael Garland's demo application called "QSlim"
|   (version 1.0) which intends to demonstrate an algorithm of mesh simplification based
|   on Garland and Heckbert(1997) "Surface Simplification Using Quadric Error Metrics".
|   The code of the demo is said to be in the public domain.
|   See: http://www.cs.cmu.edu/afs/cs/Web/People/garland/quadrics/qslim10.html
|
|   $Revision: 1.0 $
|       $Date: 2014/09/17 $
|     $Author: Christian.Cote $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <stdlib.h>
#include <string>

using namespace std;

enum Axis { X = 0, Y = 1, Z = 2, W = 3 };
enum ConstraintQuadricType { PLANE_CONSTRAINT, VERTEX_CONSTRAINT };
enum OptimalPlacementPolicyType { PLACE_ENDPOINTS, PLACE_ENDORMID, PLACE_LINE, PLACE_OPTIMAL };

#ifndef FEQ_EPS
#define FEQ_EPS 1e-6
#define FEQ_EPS2 1e-12
#endif
inline bool FEQ(double a, double b, double eps = FEQ_EPS) { return fabs(a - b)<eps; }
inline bool FEQ(float a, float b, float eps = FEQ_EPS) { return fabsf(a - b)<eps; }
