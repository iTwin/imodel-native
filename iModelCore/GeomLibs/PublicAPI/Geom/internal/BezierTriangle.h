/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE



struct IndexedWeights {
					  int childIndex;
					  int parentIndex;
					  double weight;
					  };

#define MAX_BEZIER_TRIANGLE_DEGREE 4
#define MAX_BEZIER_TRIANGLE_ORDER  5
#define MAX_BEZIER_TRIANGLE_POINTS 15


template <typename T>

struct GenericBezierTriangle
{
public: 
// Should be private, but I don't see how to do the specialized accessors
//s_numPoints, s_orderFP
int m_order;         
T m_controlPoints[MAX_BEZIER_TRIANGLE_POINTS];    
public:

//! @description Construct a triangle from an array of control points
//! @param [in] controlPoints The array of control points
//! @param [in] size The size of the array
GEOMDLLIMPEXP GenericBezierTriangle (T *controlPoints, int size);

//! @description Construct from 0, 1, 3, 6, 10, or 15 control points of (empty, constant, linear, quadratic, cubic, quartic) triangle.
//! @description If other number is supplied, the leading points are used, but don't expect that to be useful.
//! @param [in] controlPoints The points from which to create the triangle
GEOMDLLIMPEXP GenericBezierTriangle (bvector<T> const &controlPoints);

//! @description Construct a triangle of order 1.
//! @param [in] pt0 The control point
GEOMDLLIMPEXP GenericBezierTriangle (T pt0);

//! @description Construct a triangle of order 2.
//! @param [in] pt0 The bottom left control point
//! @param [in] pt1 The bottom right control point
//! @param [in] pt2 The top control point
GEOMDLLIMPEXP GenericBezierTriangle (T pt0, T pt1, T pt2);

//! @description Construct a triangle of order 3.
//! @param [in] pt0 The bottom left control point
//! @param [in] pt1 The bottom center control point
//! @param [in] pt2 The bottom right control point
//! @param [in] pt3 The midway left control point
//! @param [in] pt4 The midway right control point
//! @param [in] pt5 The top control point
GEOMDLLIMPEXP GenericBezierTriangle (T pt0, T pt1, T pt2, T pt3, T pt4, T pt5);

//! @description Construct a triangle of order 4.
//! @param [in] pt0 The bottom left control point
//! @param [in] pt1 The bottom center-left control point
//! @param [in] pt2 The bottom center-right control point
//! @param [in] pt3 The bottom right control point
//! @param [in] pt4 The lower midway left control point
//! @param [in] pt5 The lower midway center control point
//! @param [in] pt6 The lower midway right control point
//! @param [in] pt7 The upper midway left control point
//! @param [in] pt8 The upper midway right control point
//! @param [in] pt9 The top control point
GEOMDLLIMPEXP GenericBezierTriangle (T pt0, T pt1, T pt2, T pt3, T pt4, T pt5,
									 T pt6, T pt7, T pt8, T pt9);

//! @description Construct an empty (order==0) triangle structure.  
GEOMDLLIMPEXP GenericBezierTriangle ();

//! @description Does nothing
//! @param [in] u The u-parameter for evaluation
//! @param [in] v The v-parameter for evaluation
void GEOMDLLIMPEXP NoOp (double u, double v);

//! @description Gives the order of the triangle
//! @return The order of the triangle
int GEOMDLLIMPEXP GetOrder () const;

//! @description Gives the degree of the triangle
//! @return The degree of the triangle
int GEOMDLLIMPEXP GetDegree () const;

//! @description Gives the number of control points in the triangle
//! @return The number of control points in the triangle	
int GEOMDLLIMPEXP GetNumberPoints () const;

//! @description Sets the order of the triangle
//! @param [in] The new order of the triangle
void GEOMDLLIMPEXP SetOrder (int i);

//! @description Gives the control point corresponding to the index number of the triangle
//! @param [in] i The index number of the control point
//! @return The desired control point
T GEOMDLLIMPEXP GetPole (int i) const;

//! @description Gives the control point corresponding to the index number in the triangle
//! @param [in] i The column position
//! @param [in] j The row position
//! @return The desired control point
T GEOMDLLIMPEXP GetPoleR (int i, int j) const;

//! @description Gives the control point corresponding to the index number in the triangle
//! @param [in] i The column position
//! @param [in] j The row position
//! @param [out] cpnt The desired control point
//! @return True or false, depending on if the index provided corresponds to an actual control point
bool GEOMDLLIMPEXP TryGetPole (int i, int j, T &cpnt) const;	

//! @description Sets the pole number in the triangle equal to a new control point
//! @param [in] i The index number of the control point
//! @param [in] cpnt The new control point value
void GEOMDLLIMPEXP SetPole (int i, T cpnt);

//! @description Sets the pole number in the triangle equal to a new control point
//! @param [in] i The column position
//! @param [in] j The row position
//! @param [in] cpnt The new control point value
void GEOMDLLIMPEXP SetPole (int i, int j, T cpnt);

//! @description Adds control point of type T to current value of control point
//! @param [in] i The index number of the point
//! @param [in] cpnt The control point to add
//! @param [in] scale THe number by which to scale the control point being added
void GEOMDLLIMPEXP AddToPole (int i, T cpnt, double scale);

//! @description Sets each entry of each control equal to zero
void GEOMDLLIMPEXP Zero ();

//! @description Sets each entry of each control equal to zero, but also forces triangles order
//! @param [in] n The order to make the triangle
void GEOMDLLIMPEXP ZeroSafe (int n);

//! @description Raises the order of the triangle by one 
//! @return The new raised-order triangle
GenericBezierTriangle GEOMDLLIMPEXP RaiseOrder () const;

//! @description Raises the order of the triangle by an integer
//! @param [in] order The new order of the triangle
//! @return The new raised-order triangle
GenericBezierTriangle GEOMDLLIMPEXP RaiseOrderTo (int order) const;

//! @description Given the Barycentric coordinates of a control point, computes its values as a T
//! @description Uses precalculated basis functions
//! @param [in] u The u-parameter for evaluation
//! @param [in] v The v-parameter for evaluation
//! @return The desired control point of type T
T GEOMDLLIMPEXP EvaluateDirect (double u, double v) const;

//! @description Given the Barycentric coordinates of a control point, computes its values as a T,
//! @description as well as the derivatives of the triangle wrt u and v.
//! @description Uses precalculated basis functions
//! @param [in] u The u-parameter for evaluation
//! @param [in] v The v-parameter for evaluation
//! @param [out] cpnt The desired control point of type T
//! @param [out] ddu The partial derivative of the triangle with respect to u
//! @param [out] ddv The partial derivative of the triangle with respect to v
void GEOMDLLIMPEXP EvaluateDirect (double u, double v, T &cpnt, T &ddu, T &ddv) const;

//! @description Given the Barycentric coordinates of a control point, computes its values as a T
//! @description Uses algorithm to compute basis functions
//! @param [in] u The u-parameter for evaluation
//! @param [in] v The v-parameter for evaluation
//! @return The desired control point of type T
T GEOMDLLIMPEXP EvaluateDirectCompact (double u, double v) const;

//! @description Given the Barycentric coordinates of a control point, computes its values as a T,
//! @description as well as the derivatives of the triangle wrt u and v.
//! @description Uses algorithm to compute basis functions
//! @param [in] u The u-parameter for evaluation
//! @param [in] v The v-parameter for evaluation
//! @param [out] cpnt The desired control point of type T
//! @param [out] ddu The partial derivative of the triangle with respect to u
//! @param [out] ddv The partial derivative of the triangle with respect to v
void GEOMDLLIMPEXP EvaluateDirectCompact (double u, double v, T &cpnt, T &ddu, T &ddv) const;

//! @description Compute basis functions for triangle
//! @description Uses precalculated basis functions
//! @param [in] u The u-parameter for evaluation
//! @param [in] v The v-parameter for evaluation
//! @param [out] values The array of basis functions
void GEOMDLLIMPEXP ComputeBasisFunctions (double u, double v, double *values) const;

//! @description Compute basis functions for triangle and their derivatives
//! @description Uses precalculated basis functions
//! @param [in] u The u-parameter for evaluation
//! @param [in] v The v-parameter for evaluation
//! @param [out] values The array of basis functions
//! @param [out] ddu The array derivatives wrt u
//! @param [out] ddv The array derivatives wrt v
void GEOMDLLIMPEXP ComputeBasisFunctions (double u, double v, double *values, double *ddu, double *ddv) const;
	
//! @description Compute basis functions for triangle
//! @description Uses algorithm to compute basis functions
//! @param [in] u The u-parameter for evaluation
//! @param [in] v The v-parameter for evaluation
//! @param [out] values The array of basis functions
void GEOMDLLIMPEXP ComputeBasisFunctionsCompact (double u, double v, double *values) const;

//! @description Compute basis functions for triangle and their derivatives
//! @description Uses algorithm to compute basis functions
//! @param [in] u The u-parameter for evaluation
//! @param [in] v The v-parameter for evaluation
//! @param [out] values The array of basis functions
//! @param [out] ddu The array derivatives wrt u
//! @param [out] ddv The array derivatives wrt v
void GEOMDLLIMPEXP ComputeBasisFunctionsCompact (double u, double v, double *values, double *ddu, double *ddv) const;
      
//! @description Run the de Casteljau subdivision on the triangle 
//! @description This is inplace and reduces the degree
//! @param [in] u The u-parameter for evaluation
//! @param [in] v The v-parameter for evaluation
void GEOMDLLIMPEXP InplaceDeCasteljauLayer (double u, double v);

//! @description Run de Casteljau layering on triangle. Produces final triangles of same degree as original.
//! @param [in] u The u-parameter for evaluation
//! @param [in] v The v-parameter for evaluation
//! @param [out] bottom The bottom triangle produced
//! @param [out] right The right triangle produced
//! @param [out] left The left triangle produced
void GEOMDLLIMPEXP LayerTriangles (double u, double v, GenericBezierTriangle &bottom, GenericBezierTriangle &right, GenericBezierTriangle &left);

//! @description Given the Barycentric coordinates of a control point,
//! @description computes its values as a T using the de Casteljau algorithm 
//! @param [in] u The u-parameter for evaluation
//! @param [in] v The v-parameter for evaluation
//! @return The desired control point of type T
T GEOMDLLIMPEXP LayerTrianglesEvaluate (double u, double v) const;

//! @description Given the Barycentric coordinates of a control point, computes its values as a T
//! @description as well as the derivatives of the triangle wrt u and v using the de Casteljau algorithm 
//! @param [in] u The u-parameter for evaluation
//! @param [in] v The v-parameter for evaluation
//! @param [out] cpnt The desired control point of type T
//! @param [out] ddu The partial derivative of the triangle with respect to u
//! @param [out] ddv The partial derivative of the triangle with respect to v
void GEOMDLLIMPEXP LayerTrianglesEvaluate (double u, double v, T &cpnt, T &ddu, T &ddv) const;

//! @description Using multiple calls of LayerTriangles, produces a triangle mesh from 4 triangles
//! @param [out] triangle00 The left->right triangle
//! @param [out] triangle10 The right->left triangle
//! @param [out] triangle01 The right->right->right triangle
//! @param [out] trianglecenter The right->right->left triangle
void GEOMDLLIMPEXP SymmetricSplit (GenericBezierTriangle &triangle00, GenericBezierTriangle &triangle10, GenericBezierTriangle &triangle01, GenericBezierTriangle &trianglecenter);

//! @description Using precalculated values, produces a triangle mesh from 4 triangles
//! @param [out] triangle00 The left->right triangle
//! @param [out] triangle10 The right->left triangle
//! @param [out] triangle01 The right->right->right triangle
//! @param [out] trianglecenter The right->right->left triangle
void GEOMDLLIMPEXP SymmetricSplitDirect (GenericBezierTriangle &triangle00, GenericBezierTriangle &triangle10, GenericBezierTriangle &triangle01, GenericBezierTriangle &trianglecenter);

//! @description Sums scaled entries of triangle according to indices defined by 'weights' to create new triangles
//! @param[in] weights The array of IndexedWeights values, each of which have 3 entries: childIndex, parentIndex, and weight
//! @param[in] numWeights The length of the array of weights
//! @param[out] child The triangle that results from running the sum
void GEOMDLLIMPEXP ComputeWeighedChildTriangle (GenericBezierTriangle &child, IndexedWeights *weights, int numWeights);

//void GEOMDLLIMPEXP TriangleDPoint3dFrom (GenericBezierTriangle<T> triangle, GenericBezierTriangle<DPoint3d> &trianglen);
//void GEOMDLLIMPEXP TriangleDFrom (GenericBezierTriangle<T> triangle, GenericBezierTriangle<double> &trianglen);
//void GEOMDLLIMPEXP TriangleOfTrianglesFrom (GenericBezierTriangle<T> triangle, GenericBezierTriangle<GenericBezierTriangle<double>> &trianglen);

//#ifdef expandAddToPole
//		 double w = 0.0;
//		 for (int i = 0; i < numWeights; i++)
//             {
//             int childIndex = weights[i].childIndex;
//             int parentIndex = weights[i].parentIndex;
//			 w = weights[i].weight;
//			 //This is about 5% faster
//			 child.m_controlPoints[childIndex].x += m_controlPoints[parentIndex].x * w;
//			 child.m_controlPoints[childIndex].y += m_controlPoints[parentIndex].y * w;
//			 child.m_controlPoints[childIndex].z += m_controlPoints[parentIndex].z * w;
//			 }
//
//#else 
//		  int childIndex [120], parentIndex [120]; 
//		  double wIndex [120];
//			
//		  for (int i = 0; i < numWeights; i++)
//             {
//             childIndex[i] = weights[i].childIndex;
//             parentIndex[i] = weights[i].parentIndex;
//			 wIndex[i] = weights[i].weight;
//			 }
//			 AddWeighted (child.m_controlPoints, childIndex, m_controlPoints, parentIndex, wIndex, numWeights);
//#endif
}; 





struct GenericBezierTriangleDPoint2d : public GenericBezierTriangle <DPoint2d>
{
//! @description Construct a triangle from a DPoint2d array
//! @param [in] controlPoints The array of control points
//! @param [in] size The size of the array
GEOMDLLIMPEXP GenericBezierTriangleDPoint2d (DPoint2d *controlPoints, int size);

//! @description Construct from 0, 1, 3, 6, 10, or 15 control points of (empty, constant, linear, quadratic, cubic, quartic) triangle.
//! @description If other number is supplied, the leading points are used, but don't expect that to be useful.
//! @param [in] controlPoints The points from which to create the triangle
GEOMDLLIMPEXP GenericBezierTriangleDPoint2d (bvector<DPoint2d> const &controlPoints);

//! @description Construct an empty (order==0) triangle structure with DPoint2d control points.  
GEOMDLLIMPEXP GenericBezierTriangleDPoint2d ();    

//! @description Copy constructor to promote the templatized class to a GenericBezierTriangleDPoint3d 
//! @param [in] source The triangle to be copied
GEOMDLLIMPEXP GenericBezierTriangleDPoint2d (GenericBezierTriangle <DPoint2d> const &source);

//! @description Set templatized class equal to a GenericBezierTriangleDPoint2d 
//! @param [in] source The triangle to be copied
GenericBezierTriangleDPoint2d GEOMDLLIMPEXP operator = (GenericBezierTriangle<DPoint2d> const & source);

//! @description Calculates the area of a triangle
//! @return The area of the triangle
double GEOMDLLIMPEXP ControlPolygonArea () const;

//! @description Transforms the triangle
//! @param [in] transform The transform matrix by which the triangle's points are multiplied
void GEOMDLLIMPEXP Multiply (TransformCR transform);

//! @description Creates a new triangle which is the transformed input triangle
//! @param [in] transform The transform matrix by which the triangle's points are multiplied
//! @param [in] triangle The triangle to be transformed
//! @return The transformed triangle
GenericBezierTriangleDPoint2d GEOMDLLIMPEXP FromMultiply (TransformCR transform, GenericBezierTriangleDPoint2d &triangle);
};


struct GenericBezierTriangleDPoint3d : public GenericBezierTriangle <DPoint3d>
{
//! @description Construct a triangle from a DPoint3d array
//! @param [in] controlPoints The array of control points
//! @param [in] size The size of the array
GEOMDLLIMPEXP GenericBezierTriangleDPoint3d (DPoint3d *controlPoints, int size);

//! @description Construct from 0, 1, 3, 6, 10, or 15 control points of (empty, constant, linear, quadratic, cubic, quartic) triangle.
//! @description If other number is supplied, the leading points are used, but don't expect that to be useful.
//! @param [in] controlPoints The points from which to create the triangle
GEOMDLLIMPEXP GenericBezierTriangleDPoint3d (bvector<DPoint3d> const &controlPoints);

//! @description Construct an empty (order==0) triangle structure with DPoint3d control points.  
GEOMDLLIMPEXP GenericBezierTriangleDPoint3d ();    

//! @description Copy constructor to promote the templatized class to a GenericBezierTriangleDPoint3d 
//! @param [in] source The triangle to be copied
GEOMDLLIMPEXP GenericBezierTriangleDPoint3d (GenericBezierTriangle <DPoint3d> const &source);

//! @description Set templatized class equal to a GenericBezierTriangleDPoint3d 
//! @param [in] source The triangle to be copied
GenericBezierTriangleDPoint3d GEOMDLLIMPEXP operator = (GenericBezierTriangle<DPoint3d> const & source);

//! @description Calculates the area of a triangle
//! @return The area of the triangle
double GEOMDLLIMPEXP ControlPolygonArea () const;

//! @description Transforms the triangle
//! @param [in] transform The transform matrix by which the triangle's points are multiplied
void GEOMDLLIMPEXP Multiply (TransformCR transform);

//! @description Creates a new triangle which is the transformed input triangle
//! @param [in] transform The transform matrix by which the triangle's points are multiplied
//! @param [in] triangle The triangle to be transformed
//! @return The transformed triangle
GenericBezierTriangleDPoint3d GEOMDLLIMPEXP FromMultiply (TransformCR transform, GenericBezierTriangleDPoint3d &triangle);

//! @description Create array of products of derivative with difference of points and make a triangle from it
//! @param [in] direction The direction in which to differentiate: u=0,v=1
//! @return A triangle of one order lower whose points are corresponding derivatives
GenericBezierTriangleDPoint3d GEOMDLLIMPEXP EvaluateDirectionalDifference (int direction);

//! @description Gets the points belong to an edge (bottom = 0, right = 1, left = 2) and
//! @description puts them into a curve.
//! @param [in] sideNumber The edge number
//! @param [in] lineIndex The depth with which to go through that side
//! @param [out] curve The resulting curve
//! @return True or false, depending on whether such an edge and depth can be accessed
bool GEOMDLLIMPEXP TryGetPoleSection (int sideNumber, int lineIndex, BezierCurveDPoint3d &curve);
};

struct GenericBezierTriangleDPoint4d : public GenericBezierTriangle <DPoint4d>
{
//! @description Construct a triangle from a DPoint4d array
//! @param [in] controlPoints The array of control points
//! @param [in] size The size of the array
GEOMDLLIMPEXP GenericBezierTriangleDPoint4d (DPoint4d *controlPoints, int size);

//! @description Construct from 0, 1, 3, 6, 10, or 15 control points of (empty, constant, linear, quadratic, cubic, quartic) triangle.
//! @description If other number is supplied, the leading points are used, but don't expect that to be useful.
//! @param [in] controlPoints The points from which to create the triangle
GEOMDLLIMPEXP GenericBezierTriangleDPoint4d (bvector<DPoint4d> const &controlPoints);

//! @description Construct an empty (order==0) triangle structure with DPoint4d control points.  
GEOMDLLIMPEXP GenericBezierTriangleDPoint4d ();    

//! @description Copy constructor to promote the templatized class to a GenericBezierTriangleDPoint4d 
//! @param [in] source The triangle to be copied
GEOMDLLIMPEXP GenericBezierTriangleDPoint4d (GenericBezierTriangle <DPoint4d> const &source);

//! @description Set templatized class equal to a GenericBezierTriangleDPoint4d 
//! @param [in] source The triangle to be copied
GenericBezierTriangleDPoint4d GEOMDLLIMPEXP operator = (GenericBezierTriangle<DPoint4d> const & source);

//! @description Calculates the area of a triangle
//! @return The area of the triangle
double GEOMDLLIMPEXP ControlPolygonArea () const;

//! @description Transforms the triangle
//! @param [in] transform The transform matrix by which the triangle's points are multiplied
void GEOMDLLIMPEXP Multiply (TransformCR transform);

//! @description Creates a new triangle which is the transformed input triangle
//! @param [in] transform The transform matrix by which the triangle's points are multiplied
//! @param [in] triangle The triangle to be transformed
//! @return The transformed triangle
GenericBezierTriangleDPoint4d GEOMDLLIMPEXP FromMultiply (TransformCR transform, GenericBezierTriangleDPoint4d &triangle);
};

struct GenericBezierTriangleD : public GenericBezierTriangle <double>
{
//! @description Construct a triangle from a double array
//! @param [in] controlPoints The array of control points
//! @param [in] size The size of the array
GEOMDLLIMPEXP GenericBezierTriangleD (double *controlPoints, int size);

//! @description Construct from 0, 1, 3, 6, 10, or 15 control points of (empty, constant, linear, quadratic, cubic, quartic) triangle.
//! @description If other number is supplied, the leading points are used, but don't expect that to be useful.
//! @param [in] controlPoints The points from which to create the triangle
GEOMDLLIMPEXP GenericBezierTriangleD (bvector<double> const &controlPoints);

//! @description Construct an empty (order==0) triangle structure with double control points.  
GEOMDLLIMPEXP GenericBezierTriangleD();

//! @description Copy constructor to promote the templatized class to a GenericBezierTriangleD 
//! @param [in] source The triangle to be copied
GEOMDLLIMPEXP GenericBezierTriangleD (GenericBezierTriangle <double> const &source);

//! @description Set templatized class equal to a GenericBezierTriangleD
//! @param [in] source The triangle to be copied
GenericBezierTriangleD GEOMDLLIMPEXP operator = (GenericBezierTriangle<double> const & source);

//! @description Calculates the area of a triangle
//! @return The area of the triangle
double GEOMDLLIMPEXP ControlPolygonArea () const;
};

struct GenericBezierTriangleOfTriangles : public GenericBezierTriangle <GenericBezierTriangleD>
{
//! @description Construct a triangle from a GenericBezierTriangleD array
//! @param [in] controlPoints The array of control points
//! @param [in] size The size of the array
GEOMDLLIMPEXP GenericBezierTriangleOfTriangles (GenericBezierTriangleD *controlPoints, int size);

//! @description Construct from 0, 1, 3, 6, 10, or 15 control points of (empty, constant, linear, quadratic, cubic, quartic) triangle.
//! @description If other number is supplied, the leading points are used, but don't expect that to be useful.
//! @param [in] controlPoints The points from which to create the triangle
GEOMDLLIMPEXP GenericBezierTriangleOfTriangles (bvector<GenericBezierTriangleD> const &controlPoints);

//! @description Construct an empty (order==0) triangle structure with GenericBezierTriangleD control points.  
GEOMDLLIMPEXP GenericBezierTriangleOfTriangles ();

//! @description Copy constructor to promote the templatized class to a GenericBezierTriangleOfTriangles
//! @param [in] source The triangle to be copied
GEOMDLLIMPEXP GenericBezierTriangleOfTriangles (GenericBezierTriangle <GenericBezierTriangleD> const &source);

//! @description Set templatized class equal to a GenericBezierTriangleOfTriangles 
//! @param [in] source The triangle to be copied
GenericBezierTriangleOfTriangles GEOMDLLIMPEXP operator = (GenericBezierTriangle<GenericBezierTriangleD> const & source);

//! @description Calculates the area of a triangle
//! @return The area of the triangle
double GEOMDLLIMPEXP ControlPolygonArea () const;
};



#define MAX_BEZIER_CURVE_DEGREE_ForStruct 4
#define MAX_BEZIER_CURVE_ORDER_ForStruct  5
#define MAX_BEZIER_CURVE_POINTS_ForStruct 5
//! BezierCurveDPoint3d carries bezier curves of degree up to MaxBezierCurveDegree 4
//! This implementation has fixed array for 5 points (degree 4) to facilitate internal use in calculations.
//! The "order" of the triangle is the total number of points, i.e. one more than the degree.
//! <pre>
//! order   degree    points
//! 0       undefined   0
//! 1       0           1
//! 2       1           2
//! 3       2           3
//! 4       3           4
//! 5       4           5
//! </pre>
struct BezierCurveDPoint3d
    {
    private:
    int  m_order;
    DPoint3d m_controlPoints[MAX_BEZIER_CURVE_POINTS_ForStruct];
	//! Construct empy n-order curve
	BezierCurveDPoint3d (int order);
    public:
    
    // STANDARD CONSTRUCTORS
    
    //! @description Construct an empty curve
    GEOMDLLIMPEXP BezierCurveDPoint3d ();
    //! @description Construct a single point curve
	//! @param[in] xyz0 The single point
    GEOMDLLIMPEXP BezierCurveDPoint3d (DPoint3dCR xyz0);
	//! @description Construct a linear curve
	//! @param[in] xyz0 The start point
	//! @param[in] xyz1 The end point
	GEOMDLLIMPEXP BezierCurveDPoint3d (DPoint3dCR xyz0, DPoint3dCR xyz1);
    //! @description Construct a quadratic curve
	//! @param[in] xyz0 The start point
	//! @param[in] xyz1 The second point
	//! @param[in] xyz2 The end point
    GEOMDLLIMPEXP BezierCurveDPoint3d (DPoint3dCR xyz0, DPoint3dCR xyz1, DPoint3dCR xyz2);
    //! @description Construct a cubic curve
	//! @param[in] xyz0 The start point
	//! @param[in] xyz1 The second point
	//! @param[in] xyz2 The third point
	//! @param[in] xyz3 The end point
    GEOMDLLIMPEXP BezierCurveDPoint3d (DPoint3dCR xyz0, DPoint3dCR xyz1, DPoint3dCR xyz2, DPoint3dCR xyz3);
   //! @description Construct a quartic curve
	//! @param[in] xyz0 The start point
	//! @param[in] xyz1 The second point
	//! @param[in] xyz2 The third point
	//! @param[in] xyz3 The fourth point
	//! @param[in] xyz4 The end point
   GEOMDLLIMPEXP BezierCurveDPoint3d (DPoint3dCR xyz0, DPoint3dCR xyz1, DPoint3dCR xyz2, DPoint3dCR xyz3, DPoint3dCR xyz4);

	//! @description Construct a Bezier curve from an array of points
	//! @param [in] points The array of points
	//! @param [in] size The size of the array
	GEOMDLLIMPEXP BezierCurveDPoint3d (DPoint3d *points, int size);

	//! @description Construct a Bezier curve from an array of vectors
	//! @param [in] points The array of vectors
	//! @param [in] size The size of the array
	GEOMDLLIMPEXP BezierCurveDPoint3d (DVec3d *points, int size);
    
	//! @description Gives the order of the curve
	//! @return The order of the curve
    int GEOMDLLIMPEXP GetOrder () const;

	//! @description Sets the order of the curve
	//! @param [in] The new order of the curve
	void GEOMDLLIMPEXP SetOrder (int i);

	//! @description Gives the degree of the curve
	//! @return The degree of the curve
    int GEOMDLLIMPEXP GetDegree () const;

	//! @description Gives the number of points in the curve
	//! @return The number of points in the curve
    int GEOMDLLIMPEXP GetNumberPoints () const;

	//! @description Gives the point corresponding to the index number of the curve
	//! @param [in] i The index number of the point
	//! @return The desired point
	DPoint3d GEOMDLLIMPEXP GetPole (int i) const;

	//! @description Gives the point corresponding to the index number of the curve, and
	//! @description in the case of a call to an index outisde of the curve, returns (0,0,0)
	//! @param [in] i The index number of the point
	//! @return The desired point
	DPoint3d GEOMDLLIMPEXP GetPoleSafe (int i) const;

	//! @description Sets the pole number in the curve to a new point
	//! @param [in] i The index number of the point
	//! @param [in] xyz The new point value
	void GEOMDLLIMPEXP SetPole (int i, DPoint3dCR xyz);

	//! @description Raises the order of the curve by one 
	//! @return The new raised-order curve
	BezierCurveDPoint3d GEOMDLLIMPEXP RaiseOrder () const;
	
	//! @description Raises the order of the curve by an integer
	//! @param [in] order The number by which to raise the order
	//! @return The new raised-order curve
	BezierCurveDPoint3d GEOMDLLIMPEXP RaiseOrderTo (int order) const;

	//! @description Does nothing
	//! @param [in] u The u-parameter for evaluation
	void GEOMDLLIMPEXP NoOp (double u);

	//! @description Transforms the curve
	//! @param [in] transform The transform matrix by which the curve's points are multiplied
	void GEOMDLLIMPEXP Multiply (TransformCR transform);

	//! @description Creates a new curve which is the transformed input curve
	//! @param [in] transform The transform matrix by which the curve's points are multiplied
	//! @param [in] curve The curve to be transformed
	//! @return The transformed curve
	BezierCurveDPoint3d GEOMDLLIMPEXP FromMultiply (TransformCR transform, BezierCurveDPoint3dCR curve);

    //! @description Construct from 0, 1, 2, 3, 4, or 5 control points of (empty, constant, linear, quadratic, cubic, quartic) curve
	//! @param [in] controlPoints The points from which to create the curve
    GEOMDLLIMPEXP BezierCurveDPoint3d (bvector<DPoint3d> const &controlPoints);
	
	//! @description Compute Cartesian coordinates of a point in the curve given parametric coordinates
	//! @description Uses precalculated basis functions
	//! @param [in] u The u-parameter for evaluation
	//! @return The desired point in Cartesian coordinates
	DPoint3d GEOMDLLIMPEXP EvaluateDirect (double u) const;

	//! @description Compute Cartesian coordinates of a point in the curve given parametric coordinates
	//! @description Uses algorithm to compute basis functions
	//! @param [in] u The u-parameter for evaluation
	//! @return The desired point in Cartesian coordinates
	DPoint3d GEOMDLLIMPEXP EvaluateDirectCompact (double u) const;

	//! @description Compute Cartesian coordinates of a point in the curve given parametric coordinates,
	//! @description as well as the derivatives of the curve wrt u and v.
	//! @description Uses precalculated basis functions
	//! @param [in] u The u-parameter for evaluation
	//! @param [out] xyz The desired point in Cartesian coordinates
	//! @param [out] dXdu The derivative of the curve with respect to u
	void GEOMDLLIMPEXP EvaluateDirect (double u, DPoint3dR xyz, DVec3dR dXdu) const;

	//! @description Compute Cartesian coordinates of a point in the curve given parametric coordinates,
	//! @description as well as the derivatives of the curve wrt u and v.
	//! @description Uses algorithm to compute basis functions
	//! @param [in] u The u-parameter for evaluation
	//! @param [out] xyz The desired point in Cartesian coordinates
	//! @param [out] dXdu The derivative of the curve with respect to u
	void GEOMDLLIMPEXP EvaluateDirectCompact (double u, DPoint3dR xyz, DVec3dR dXdu) const;

	//! @description Compute basis functions for curve
	//! @description Uses precalculated basis functions
	//! @param [in] u The u-parameter for evaluation
	//! @param [out] values The array of basis functions
	void GEOMDLLIMPEXP ComputeBasisFunctions (double u, double *values) const;

	//! @description Compute basis functions for curve and their derivatives
	//! @description Uses precalculated basis functions
	//! @param [in] u The u-parameter for evaluation
	//! @param [out] values The array of basis functions
	//! @param [out] dXdu The array derivatives wrt u
	void ComputeBasisFunctions (double u, double *values, double *dXdu) const;
	
	//! @description Compute basis functions for curve
	//! @description Uses algorithm to compute basis functions
	//! @param [in] u The u-parameter for evaluation
	//! @param [out] values The array of basis functions
	void GEOMDLLIMPEXP ComputeBasisFunctionsCompact (double u, double *values) const;
	
	//! @description Compute basis functions for curve and their derivatives
	//! @description Uses algorithm to compute basis functions
	//! @param [in] u The u-parameter for evaluation
	//! @param [out] values The array of basis functions
	//! @param [out] dXdu The array derivatives wrt u
	void GEOMDLLIMPEXP ComputeBasisFunctionsCompact (double u, double *values, double *ddu) const;

	//! @description Run the de Casteljau subdivision on the curve
	//! @description This is inplace and reduces the degree
	//! @param [in] u The u-parameter for evaluation
    void GEOMDLLIMPEXP InplaceDeCasteljau (double u);

    //! @description Construct from 0, 1, 2, 3, 4, or 5 control points of (empty, constant, linear, quadratic, cubic, quartic) curve
	//! @param [in] controlPoints The points from which to create the curve
    //! @return the order actually installed
    int GEOMDLLIMPEXP SetControlPoints (bvector<DPoint3d> const &controlPoints);   

	//! @description Gets a count of the number of facets in the curve
	//! @param [in] chordTolerance The chord tolerance
	//! @param [in] angleTolerance The angle tolerance
	//! @param [in] maxEdgeLength The maximum edge length
	int GEOMDLLIMPEXP GetFacetCount (double chordTolerance, double angleTolerance, double maxEdgeLength);

	//! Evaluate at u,v and return the point.
    //DPoint3d GEOMDLLIMPEXP Evaluate (double u, double v) const;

    };




END_BENTLEY_GEOMETRY_NAMESPACE