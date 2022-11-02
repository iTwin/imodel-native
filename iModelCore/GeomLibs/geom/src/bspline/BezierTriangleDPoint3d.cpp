/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
struct RefCountedBezierTriangleDPoint3d;
typedef RefCountedPtr <RefCountedBezierTriangleDPoint3d> BezierTriangleDPoint3dPtr;



#define MAX_BEZIER_TRIANGLE_DEGREE 4
#define MAX_BEZIER_TRIANGLE_ORDER  5
#define MAX_BEZIER_TRIANGLE_POINTS 15

//! BezierTriangleDPoint3d carries bezier triangle patches of degree up to MaxBezierTriangleDegree 4
//! This implementation has fixe array for 15 points (degree 4) to facilitate internal use in calculations.
//! The "order" of the triangle is the number of points along an edge, i.e. one more than the degree.
//! <pre>
//! order   degree    points
//! 0       undefined   0
//! 1       0           1
//! 2       1           3
//! 3       2           6
//! 4       3           10
//! 5       4           15
//! </pre>
struct BezierTriangleDPoint3d
    {
    private:
    int  m_order;
    DPoint3d m_controlPoints[MAX_BEZIER_TRIANGLE_POINTS];
	//! Construct empy n-order triangle
	BezierTriangleDPoint3d (int order);
    public:
    
    // STANDARD CONSTRUCTORS
    
	//! @description Construct an empty (degree==-1) triangle structure.  
    GEOMDLLIMPEXP BezierTriangleDPoint3d ();
	//! @description Construct a single point triangle.
	//! @param [in] xyz The single point
    GEOMDLLIMPEXP BezierTriangleDPoint3d (DPoint3dCR xyz);
	//! @description Construct a linear triangle.
	//! @param [in] xyz00 The bottom left point
	//! @param [in] xyz10 The bottom right point
	//! @param [in] xyz01 The top point
    GEOMDLLIMPEXP BezierTriangleDPoint3d (DPoint3dCR xyz00, DPoint3dCR xyz10, DPoint3dCR xyz01);
    //! @description Construct a quadratic triangle.
	//! @param [in] xyz00 The bottom left point
	//! @param [in] xyz10 The bottom center point
	//! @param [in] xyz21 The bottom right point
	//! @param [in] xyz01 The middle left point
	//! @param [in] xyz11 The middle right point
	//! @param [in] xyz02 The top point
    GEOMDLLIMPEXP BezierTriangleDPoint3d (
            DPoint3dCR xyz00, DPoint3dCR xyz10, DPoint3dCR xyz21,
            DPoint3dCR xyz01, DPoint3dCR xyz11,
            DPoint3dCR xyz02
            );
	//! @description Construct a cubic triangle.
	//! @param [in] xyz00,xyz10,xyz20,xyz30 The first row of points
	//! @param [in] xyz01,xyz11,xyz21 The second row of points
	//! @param [in] xyz02,xyz12 The third row of points
	//! @param [in] xyz03 The fourth row of points
    GEOMDLLIMPEXP BezierTriangleDPoint3d (
            DPoint3dCR xyz00, DPoint3dCR xyz10, DPoint3dCR xyz20, DPoint3d xyz30,
            DPoint3dCR xyz01, DPoint3dCR xyz11, DPoint3dCR xyz21,
            DPoint3dCR xyz02, DPoint3dCR xyz12,
            DPoint3dCR xyz03
            );

	//! @description Construct a Bezier triangle from an array of points
	//! @param [in] points The array of points
	//! @param [in] size The size of the array
	GEOMDLLIMPEXP BezierTriangleDPoint3d (DPoint3d *points, int size);

	//! @description Construct a Bezier triangle from an array of vectors
	//! @param [in] points The array of vectors
	//! @param [in] size The size of the array
	GEOMDLLIMPEXP BezierTriangleDPoint3d (DVec3d *points, int size);
 
	//! @description Gives the order of the triangle
	//! @return The order of the triangle
    int GEOMDLLIMPEXP GetOrder () const;

	//! @description Gives the degree of the triangle
	//! @return The degree of the triangle
    int GEOMDLLIMPEXP GetDegree () const;

	//! @description Gives the number of points in the triangle
	//! @return The number of points in the triangle
    int GEOMDLLIMPEXP GetNumberPoints () const;

	//! @description Gives the point corresponding to the index number of the triangle
	//! @param [in] i The index number of the point
	//! @return The desired point
	DPoint3d GEOMDLLIMPEXP GetPole (int i) const;

	//! @description Gives the point corresponding to the index number in the triangle
	//! @param [in] i The column position
	//! @param [in] j The row position
	//! @param [out] xyz The desired point
	//! @return True or false, depending on if the index provided corresponds to an actual point
	bool GEOMDLLIMPEXP TryGetPole (int i, int j, DPoint3dR xyz) const;

	//! @description Sets the pole number in the triangle to a new point
	//! @param [in] i The index number of the point
	//! @param [in] xyz The new point value
	void GEOMDLLIMPEXP SetPole (int i, DPoint3dCR xyz);

	//! @description Adds point to current value of pole
	//! @param [in] i The index number of the point
	//! @param [in] xyz The point to add
	void GEOMDLLIMPEXP AddToPole (int i, DPoint3dCR xyz, double scale);

	//! @description Scales pole
	//! @param [in] i The index number of the point
	//! @param [in] scale The number by which to multiply the pole's values
	void GEOMDLLIMPEXP ScalePole (int i, double scale);
	
	//! @description Sets all entries of triangle equal to zero
	void GEOMDLLIMPEXP Zero ();

	//! @description Raises the order of the triangle by one 
	//! @return The new raised-order triangle
	BezierTriangleDPoint3d GEOMDLLIMPEXP RaiseOrder () const;

	//! @description Raises the order of the triangle by an integer
	//! @param [in] order The new order of the triangle
	//! @return The new raised-order triangle
	BezierTriangleDPoint3d GEOMDLLIMPEXP RaiseOrderTo (int order) const;

	//! @description Sets the pole number in the triangle to a new point
	//! @param [in] i The column position
	//! @param [in] j The row position
	//! @param [in] xyz The new point value
	void GEOMDLLIMPEXP SetPole (int i, int j, DPoint3dCR xyz);

	//! @description Sets the pole number in the triangle to a new point
	//! @param [in] i The column position
	//! @param [in] j The row position
	//! @param [in] xyz The new point value
	void GEOMDLLIMPEXP SetPole (int i, int j, DVec3dCR xyz);

	//! @description Calculates the area of a triangle
	//! @return The area of the triangle
	double GEOMDLLIMPEXP ControlPolygonArea () const;

	//! @description Does nothing
	//! @param [in] u The u-parameter for evaluation
	//! @param [in] v The v-parameter for evaluation
	void GEOMDLLIMPEXP NoOp (double u, double v);

	//! @description Transforms the triangle
	//! @param [in] transform The transform matrix by which the triangle's points are multiplied
	void GEOMDLLIMPEXP Multiply (TransformCR transform);

	//! @description Creates a new triangle which is the transformed input triangle
	//! @param [in] transform The transform matrix by which the triangle's points are multiplied
	//! @param [in] triangle The triangle to be transformed
	//! @return The transformed triangle
	BezierTriangleDPoint3d GEOMDLLIMPEXP FromMultiply (TransformCR transform, BezierTriangleDPoint3dCR triangle);

    //! @description Construct from 0, 1, 3, 6, 10, or 15 control points of (empty, constant, linear, quadratic, cubic, quartic) triangle.
    //! @description If other number is supplied, the leading points are used, but don't expect that to be useful.
	//! @param [in] controlPoints The points from which to create the triangle
    GEOMDLLIMPEXP BezierTriangleDPoint3d (bvector<DPoint3d> const &controlPoints);

	//! @description Compute Cartesian coordinates of a point in the triangle given Barycentric coordinates
	//! @description Uses precalculated basis functions
	//! @param [in] uvw The point in Barycentric coordinates
	//! @return The desired point in Cartesian coordinates
	DPoint3d GEOMDLLIMPEXP EvaluateDirect (DPoint3dCR uvw) const;

	//! @description Compute Cartesian coordinates of a point in the triangle given Barycentric coordinates,
	//! @description Uses precalculated basis functions
	//! @param [in] u The u-parameter for evaluation
	//! @param [in] v The v-parameter for evaluation
	//! @return The desired point in Cartesian coordinates
	DPoint3d GEOMDLLIMPEXP EvaluateDirect (double u, double v) const;
	
	//! @description Compute Cartesian coordinates of a point in the triangle given Barycentric coordinates
	//! @description Uses algorithm to compute basis functions
	//! @param [in] u The u-parameter for evaluation
	//! @param [in] v The v-parameter for evaluation
	//! @return The desired point in Cartesian coordinates
	DPoint3d GEOMDLLIMPEXP EvaluateDirectCompact (double u, double v) const;

	//! @description Compute Cartesian coordinates of a point in the triangle given Barycentric coordinates,
	//! @description as well as the derivatives of the triangle wrt u and v.
	//! @description Uses precalculated basis functions
	//! @param [in] u The u-parameter for evaluation
	//! @param [in] v The v-parameter for evaluation
	//! @param [out] xyz The desired point in Cartesian coordinates
	//! @param [out] dXdu The partial derivative of the triangle with respect to u
	//! @param [out] dXdv The partial derivative of the triangle with respect to v
	void GEOMDLLIMPEXP EvaluateDirect (double u, double v, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) const;

	//! @description Compute Cartesian coordinates of a point in the triangle given Barycentric coordinates,
	//! @description as well as the derivatives of the triangle wrt u and v.
	//! @description Uses algorithm to compute basis functions
	//! @param [in] u The u-parameter for evaluation
	//! @param [in] v The v-parameter for evaluation
	//! @param [out] xyz The desired point in Cartesian coordinates
	//! @param [out] dXdu The partial derivative of the triangle with respect to u
	//! @param [out] dXdv The partial derivative of the triangle with respect to v
	void GEOMDLLIMPEXP EvaluateDirectCompact (double u, double v, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) const;

	//! @description Create array of products of derivative with difference of points and make a triangle from it
	//! @param [in] direction The direction in which to differentiate: u=0,v=1
	//! @return A Bezier triangle of one order lower whose points are corresponding derivatives
	BezierTriangleDPoint3d GEOMDLLIMPEXP EvaluateDirectionalDifference (int direction);

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
	//! @param [out] dXdu The array derivatives wrt u
	//! @param [out] dXdv The array derivatives wrt v
	void ComputeBasisFunctions (double u, double v, double *values, double *dXdu, double *dXdv) const;
	
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
	//! @param [out] dXdu The array derivatives wrt u
	//! @param [out] dXdv The array derivatives wrt v
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
	void GEOMDLLIMPEXP LayerTriangles (double u, double v, BezierTriangleDPoint3dR bottom, BezierTriangleDPoint3dR right, BezierTriangleDPoint3dR left);
	
	//! @description Using the de Casteljau algorithm, compute Cartesian coordinates of a point in the 
	//! @description triangle given Barycentric coordinates.
	//! @param [in] u The u-parameter for evaluation
	//! @param [in] v The v-parameter for evaluation
	//! @return The desired point in Cartesian coordinates
	DPoint3d GEOMDLLIMPEXP LayerTrianglesEvaluate (double u, double v) const;

	//! @description Using the de Casteljau algorithm, compute Cartesian coordinates of a point in the triangle
	//! @description given Barycentric coordinates, as well as the derivatives of the triangle wrt u and v
	//! @param [in] u The u-parameter for evaluation
	//! @param [in] v The v-parameter for evaluation
	//! @param [out] xyz The desired point in Cartesian coordinates
	//! @param [out] dXdu The partial derivative of the triangle with respect to u
	//! @param [out] dXdv The partial derivative of the triangle with respect to v
	void GEOMDLLIMPEXP LayerTrianglesEvaluate (double u, double v, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) const;

	//! @description Using multiple calls of LayerTriangles, produces a triangle mesh from 4 Bezier triangles
	//! @param [out] triangle00 The left->right triangle
	//! @param [out] triangle10 The right->left triangle
	//! @param [out] triangle01 The right->right->right triangle
	//! @param [out] trianglecenter The right->right->left triangle
	void GEOMDLLIMPEXP SymmetricSplit (BezierTriangleDPoint3dR triangle00, BezierTriangleDPoint3dR triangle10, BezierTriangleDPoint3dR triangle01, BezierTriangleDPoint3dR trianglecenter);

	//! @description Using precalculated values, produces a triangle mesh from 4 Bezier triangles
	//! @param [out] triangle00 The left->right triangle
	//! @param [out] triangle10 The right->left triangle
	//! @param [out] triangle01 The right->right->right triangle
	//! @param [out] trianglecenter The right->right->left triangle
	void GEOMDLLIMPEXP SymmetricSplitDirect (BezierTriangleDPoint3dR triangle00, BezierTriangleDPoint3dR triangle10, BezierTriangleDPoint3dR triangle01, BezierTriangleDPoint3dR trianglecenter);

	//! @description Sums scaled entries of triangle according to indices defined by 'weights' to create new triangles
	//! @param[in] weights The array of IndexedWeights values, each of which have 3 entries: childIndex, parentIndex, and weight
	//! @param[in] numWeights The length of the array of weights
	//! @param[out] child The triangle that results from running the sum
	void GEOMDLLIMPEXP ComputeWeighedChildTriangle (BezierTriangleDPoint3dR child, IndexedWeights *weights, int numWeights);

    //! Evaluate at u,v and return the point.
    DPoint3d GEOMDLLIMPEXP Evaluate (double u, double v) const;

    //! Set 0, 1, 3, 6, 10, or 15 control points of (empty, constant, linear, quadratic, cubic, quartic) triangle.
    //! If other number is supplied, the leading points are used, but don't expect that to be useful.
    //! @return the order actually installed.
    int GEOMDLLIMPEXP SetControlPoints (bvector<DPoint3d> const &controlPoints);

    // SMART POINTER CONSTRUCTORS
    

    //! @description Construct a linear triangle.
	//! @param [in] xyz00 The bottom left point
	//! @param [in] xyz10 The bottom right point
	//! @param [in] xyz01 The top point
    static GEOMDLLIMPEXP BezierTriangleDPoint3dPtr CreateLinear(DPoint3dCR xyz00, DPoint3dCR xyz10, DPoint3dCR xyz01);

    //! @description Construct a quadratic triangle.
	//! @param [in] xyz00 The bottom left point
	//! @param [in] xyz10 The bottom center point
	//! @param [in] xyz21 The bottom right point
	//! @param [in] xyz01 The middle left point
	//! @param [in] xyz11 The middle right point
	//! @param [in] xyz02 The top point
    static GEOMDLLIMPEXP BezierTriangleDPoint3dPtr CreateQuadratic (
            DPoint3dCR xyz00, DPoint3dCR xyz10, DPoint3dCR xyz21,
            DPoint3dCR xyz01, DPoint3dCR xyz11,
            DPoint3dCR xyz02
            );
    //! @description Construct a cubic triangle.
	//! @param [in] xyz00,xyz10,xyz20,xyz30 The first row of points
	//! @param [in] xyz01,xyz11,xyz21 The second row of points
	//! @param [in] xyz02,xyz12 The third row of points
	//! @param [in] xyz03 The fourth row of points
    static GEOMDLLIMPEXP BezierTriangleDPoint3dPtr CreateCubic (
            DPoint3dCR xyz00, DPoint3dCR xyz10, DPoint3dCR xyz20, DPoint3d xyz30,
            DPoint3dCR xyz01, DPoint3dCR xyz11, DPoint3dCR xyz21,
            DPoint3dCR xyz02, DPoint3dCR xyz12,
            DPoint3dCR xyz03
            );

    //! @description Construct from 3, 6, 10, or 15 control points of (linear, quadratic, cubic, quartic) triangle.
    //! @description If other number is supplied, a null is returned.
	//! @param [in] controlPoints The points from which to create the curve
    static GEOMDLLIMPEXP BezierTriangleDPoint3dPtr Create (bvector<DPoint3d> const &controlPoints);

    //! Create order 0.
    static GEOMDLLIMPEXP BezierTriangleDPoint3dPtr Create ();   
    }; // BezierTriangleDPoint3d


//! BezierTriangleDPoint3d with IRefCounted support for smart pointers.
//! Create via BezierTriangleDPoint3d::CreatePtr ();
struct RefCountedBezierTriangleDPoint3dBase : public BezierTriangleDPoint3d, IRefCounted
    {
    protected:
    RefCountedBezierTriangleDPoint3dBase ();
    ~RefCountedBezierTriangleDPoint3dBase();
    };

struct RefCountedBezierTriangleDPoint3d : public RefCounted<RefCountedBezierTriangleDPoint3dBase>
    {
    friend struct BezierTriangleDPoint3d;
    };





	struct BezierTriangleD
    {
    private:
    double m_controlPoints[15];
	int m_order;
	public:
	GEOMDLLIMPEXP BezierTriangleD ();

	GEOMDLLIMPEXP BezierTriangleD (double *points, int size);

	int GEOMDLLIMPEXP GetOrder () const;

	void GEOMDLLIMPEXP Zero (int np);

	void GEOMDLLIMPEXP AddToPole (int i, double val);

	void GEOMDLLIMPEXP SetOrder (int i);

	double GEOMDLLIMPEXP GetPole (int i) const;

	BezierTriangleD GEOMDLLIMPEXP AddScaledBez (BezierTriangleDCP points, double *scales, size_t n);
    };

struct BezierTriangleOfTriangles
    {
    private:
    BezierTriangleD m_triangles[15];
	int  m_order;
	public:
	GEOMDLLIMPEXP BezierTriangleOfTriangles ();

	GEOMDLLIMPEXP BezierTriangleOfTriangles (BezierTriangleD *points, int size);

	int GEOMDLLIMPEXP GetOrder () const;

	void GEOMDLLIMPEXP SetPole (int i, BezierTriangleD val);

	BezierTriangleD GEOMDLLIMPEXP GetPole (int i) const;

    void GEOMDLLIMPEXP InplaceDeCasteljauLayer (double u, double v);

	void GEOMDLLIMPEXP LayerTriangles (double u, double v, BezierTriangleOfTrianglesR bottom, BezierTriangleOfTrianglesR right, BezierTriangleOfTrianglesR left);

	void GEOMDLLIMPEXP SymmetricSplit (BezierTriangleOfTrianglesR triangle00, BezierTriangleOfTrianglesR triangle10, BezierTriangleOfTrianglesR triangle01, BezierTriangleOfTrianglesR trianglecenter);
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

	//! Evaluate at u,v and return the point.
    //DPoint3d GEOMDLLIMPEXP Evaluate (double u, double v) const;

    };



DPoint3d AddScaled (DPoint3dCP points, double *scales, size_t n)
	{
	DPoint3d xyz;
	xyz.Zero ();
	for (size_t i = 0; i < n; i++)
		{
		xyz.x += points[i].x * scales[i];
		xyz.y += points[i].y * scales[i];
		xyz.z += points[i].z * scales[i];
		}
	return xyz;	
	}

DPoint3d Scale (DPoint3d point, double scale)
	{
	DPoint3d xyz;
	xyz.Zero ();
	xyz.x = point.x * scale;
	xyz.y = point.y * scale;
	xyz.z = point.z * scale;
	return xyz;	
	}

static int s_fact [5] =
	{
	1,1,2,6,24
	};

static int s_trinomial [5][5][5] =
	{
	{   // DEGREE 0
            {1,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
        },
        {   // DEGREE 1
            {1,1,0,0,0},
            {1,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
        },
        {   // DEGREE 2
            {1,2,1,0,0},
            {2,2,0,0,0},
            {1,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
        },
        {   // DEGREE 3
            {1,3,3,1,0},
            {3,6,3,0,0},
            {3,3,0,0,0},
            {1,0,0,0,0},
            {0,0,0,0,0},
        },
        {   // DEGREE 4
            {1,4,6,4,1},
            {4,12,12,4,0},  
            {6,12,6,0,0},
            {4,4,0,0,0},
            {1,0,0,0,0},
        },
	};

static IndexedWeights order2LeftWeights [] =
	{
		{0,0,1.0},
		{1,0,0.5},
		{1,1,0.5},
		{2,0,0.5},
		{2,2,0.5}
	};

static IndexedWeights order2RightWeights [] =
	{
		{0,0,0.5},
		{0,1,0.5},
		{1,1,1.0},
		{2,1,0.5},
		{2,2,0.5}
	};

static IndexedWeights order2TopWeights [] =
	{
		{0,0,0.5},
		{0,2,0.5},
		{1,1,0.5},
		{1,2,0.5},
		{2,2,1.0}
	};

static IndexedWeights order2CenterWeights [] =
	{
		{0,1,0.5},
		{0,2,0.5},
		{1,0,0.5},
		{1,2,0.5},
		{2,0,0.5},
		{2,1,0.5}
	};

static IndexedWeights order3LeftWeights [] =
    {

        {0,0,1.000000},
        {1,0,0.500000},
        {1,1,0.500000},
        {2,0,0.250000},
        {2,1,0.500000},
        {2,2,0.250000},
        {3,0,0.500000},
        {3,3,0.500000},
        {4,0,0.250000},
        {4,1,0.250000},
        {4,3,0.250000},
        {4,4,0.250000},
        {5,0,0.250000},
        {5,3,0.500000},
        {5,5,0.250000}
    };

static IndexedWeights order3RightWeights [] =
    {
        {0,0,0.250000},
        {0,1,0.500000},
        {0,2,0.250000},
        {1,1,0.500000},
        {1,2,0.500000},
        {2,2,1.000000},
        {3,1,0.250000},
        {3,2,0.250000},
        {3,3,0.250000},
        {3,4,0.250000},
        {4,2,0.500000},
        {4,4,0.500000},
        {5,2,0.250000},
        {5,4,0.500000},
        {5,5,0.250000}
    };

static IndexedWeights order3TopWeights [] =
    {
        {0,0,0.250000},
        {0,3,0.500000},
        {0,5,0.250000},
        {1,1,0.250000},
        {1,3,0.250000},
        {1,4,0.250000},
        {1,5,0.250000},
        {2,2,0.250000},
        {2,4,0.500000},
        {2,5,0.250000},
        {3,3,0.500000},
        {3,5,0.500000},
        {4,4,0.500000},
        {4,5,0.500000},
        {5,5,1.000000}
    };

static IndexedWeights order3CenterWeights [] =
    {
        {0,2,0.250000},
        {0,4,0.500000},
        {0,5,0.250000},
        {1,1,0.250000},
        {1,3,0.250000},
        {1,4,0.250000},
        {1,5,0.250000},
        {2,0,0.250000},
        {2,3,0.500000},
        {2,5,0.250000},
        {3,1,0.250000},
        {3,2,0.250000},
        {3,3,0.250000},
        {3,4,0.250000},
        {4,0,0.250000},
        {4,1,0.250000},
        {4,3,0.250000},
        {4,4,0.250000},
        {5,0,0.250000},
        {5,1,0.500000},
        {5,2,0.250000}
    };

static IndexedWeights order4LeftWeights [] =
    {

        {0,0,1.000000},
        {1,0,0.500000},
        {1,1,0.500000},
        {2,0,0.250000},
        {2,1,0.500000},
        {2,2,0.250000},
        {3,0,0.125000},
        {3,1,0.375000},
        {3,2,0.375000},
        {3,3,0.125000},
        {4,0,0.500000},
        {4,4,0.500000},
        {5,0,0.250000},
        {5,1,0.250000},
        {5,4,0.250000},
        {5,5,0.250000},
        {6,0,0.125000},
        {6,1,0.250000},
        {6,2,0.125000},
        {6,4,0.125000},
        {6,5,0.250000},
        {6,6,0.125000},
        {7,0,0.250000},
        {7,4,0.500000},
        {7,7,0.250000},
        {8,0,0.125000},
        {8,1,0.125000},
        {8,4,0.250000},
        {8,5,0.250000},
        {8,7,0.125000},
        {8,8,0.125000},
        {9,0,0.125000},
        {9,4,0.375000},
        {9,7,0.375000},
        {9,9,0.125000}
    };

static IndexedWeights order4RightWeights [] =
    {
        {0,0,0.125000},
        {0,1,0.375000},
        {0,2,0.375000},
        {0,3,0.125000},
        {1,1,0.250000},
        {1,2,0.500000},
        {1,3,0.250000},
        {2,2,0.500000},
        {2,3,0.500000},
        {3,3,1.000000},
        {4,1,0.125000},
        {4,2,0.250000},
        {4,3,0.125000},
        {4,4,0.125000},
        {4,5,0.250000},
        {4,6,0.125000},
        {5,2,0.250000},
        {5,3,0.250000},
        {5,5,0.250000},
        {5,6,0.250000},
        {6,3,0.500000},
        {6,6,0.500000},
        {7,2,0.125000},
        {7,3,0.125000},
        {7,5,0.250000},
        {7,6,0.250000},
        {7,7,0.125000},
        {7,8,0.125000},
        {8,3,0.250000},
        {8,6,0.500000},
        {8,8,0.250000},
        {9,3,0.125000},
        {9,6,0.375000},
        {9,8,0.375000},
        {9,9,0.125000}
    };

static IndexedWeights order4TopWeights [] =
    {
        {0,0,0.125000},
        {0,4,0.375000},
        {0,7,0.375000},
        {0,9,0.125000},
        {1,1,0.125000},
        {1,4,0.125000},
        {1,5,0.250000},
        {1,7,0.250000},
        {1,8,0.125000},
        {1,9,0.125000},
        {2,2,0.125000},
        {2,5,0.250000},
        {2,6,0.125000},
        {2,7,0.125000},
        {2,8,0.250000},
        {2,9,0.125000},
        {3,3,0.125000},
        {3,6,0.375000},
        {3,8,0.375000},
        {3,9,0.125000},
        {4,4,0.250000},
        {4,7,0.500000},
        {4,9,0.250000},
        {5,5,0.250000},
        {5,7,0.250000},
        {5,8,0.250000},
        {5,9,0.250000},
        {6,6,0.250000},
        {6,8,0.500000},
        {6,9,0.250000},
        {7,7,0.500000},
        {7,9,0.500000},
        {8,8,0.500000},
        {8,9,0.500000},
        {9,9,1.000000}
    };

static IndexedWeights order4CenterWeights [] =
    {
        {0,3,0.125000},
        {0,6,0.375000},
        {0,8,0.375000},
        {0,9,0.125000},
        {1,2,0.125000},
        {1,5,0.250000},
        {1,6,0.125000},
        {1,7,0.125000},
        {1,8,0.250000},
        {1,9,0.125000},
        {2,1,0.125000},
        {2,4,0.125000},
        {2,5,0.250000},
        {2,7,0.250000},
        {2,8,0.125000},
        {2,9,0.125000},
        {3,0,0.125000},
        {3,4,0.375000},
        {3,7,0.375000},
        {3,9,0.125000},
        {4,2,0.125000},
        {4,3,0.125000},
        {4,5,0.250000},
        {4,6,0.250000},
        {4,7,0.125000},
        {4,8,0.125000},
        {5,1,0.125000},
        {5,2,0.125000},
        {5,4,0.125000},
        {5,5,0.250000},
        {5,6,0.125000},
        {5,7,0.125000},
        {5,8,0.125000},
        {6,0,0.125000},
        {6,1,0.125000},
        {6,4,0.250000},
        {6,5,0.250000},
        {6,7,0.125000},
        {6,8,0.125000},
        {7,1,0.125000},
        {7,2,0.250000},
        {7,3,0.125000},
        {7,4,0.125000},
        {7,5,0.250000},
        {7,6,0.125000},
        {8,0,0.125000},
        {8,1,0.250000},
        {8,2,0.125000},
        {8,4,0.125000},
        {8,5,0.250000},
        {8,6,0.125000},
        {9,0,0.125000},
        {9,1,0.375000},
        {9,2,0.375000},
        {9,3,0.125000}
    };

static IndexedWeights order5LeftWeights [] =
    {

        {0,0,1.000000},
        {1,0,0.500000},
        {1,1,0.500000},
        {2,0,0.250000},
        {2,1,0.500000},
        {2,2,0.250000},
        {3,0,0.125000},
        {3,1,0.375000},
        {3,2,0.375000},
        {3,3,0.125000},
        {4,0,0.062500},
        {4,1,0.250000},
        {4,2,0.375000},
        {4,3,0.250000},
        {4,4,0.062500},
        {5,0,0.500000},
        {5,5,0.500000},
        {6,0,0.250000},
        {6,1,0.250000},
        {6,5,0.250000},
        {6,6,0.250000},
        {7,0,0.125000},
        {7,1,0.250000},
        {7,2,0.125000},
        {7,5,0.125000},
        {7,6,0.250000},
        {7,7,0.125000},
        {8,0,0.062500},
        {8,1,0.187500},
        {8,2,0.187500},
        {8,3,0.062500},
        {8,5,0.062500},
        {8,6,0.187500},
        {8,7,0.187500},
        {8,8,0.062500},
        {9,0,0.250000},
        {9,5,0.500000},
        {9,9,0.250000},
        {10,0,0.125000},
        {10,1,0.125000},
        {10,5,0.250000},
        {10,6,0.250000},
        {10,9,0.125000},
        {10,10,0.125000},
        {11,0,0.062500},
        {11,1,0.125000},
        {11,2,0.062500},
        {11,5,0.125000},
        {11,6,0.250000},
        {11,7,0.125000},
        {11,9,0.062500},
        {11,10,0.125000},
        {11,11,0.062500},
        {12,0,0.125000},
        {12,5,0.375000},
        {12,9,0.375000},
        {12,12,0.125000},
        {13,0,0.062500},
        {13,1,0.062500},
        {13,5,0.187500},
        {13,6,0.187500},
        {13,9,0.187500},
        {13,10,0.187500},
        {13,12,0.062500},
        {13,13,0.062500},
        {14,0,0.062500},
        {14,5,0.250000},
        {14,9,0.375000},
        {14,12,0.250000},
        {14,14,0.062500}
    };

static IndexedWeights order5RightWeights [] =
    {
        {0,0,0.062500},
        {0,1,0.250000},
        {0,2,0.375000},
        {0,3,0.250000},
        {0,4,0.062500},
        {1,1,0.125000},
        {1,2,0.375000},
        {1,3,0.375000},
        {1,4,0.125000},
        {2,2,0.250000},
        {2,3,0.500000},
        {2,4,0.250000},
        {3,3,0.500000},
        {3,4,0.500000},
        {4,4,1.000000},
        {5,1,0.062500},
        {5,2,0.187500},
        {5,3,0.187500},
        {5,4,0.062500},
        {5,5,0.062500},
        {5,6,0.187500},
        {5,7,0.187500},
        {5,8,0.062500},
        {6,2,0.125000},
        {6,3,0.250000},
        {6,4,0.125000},
        {6,6,0.125000},
        {6,7,0.250000},
        {6,8,0.125000},
        {7,3,0.250000},
        {7,4,0.250000},
        {7,7,0.250000},
        {7,8,0.250000},
        {8,4,0.500000},
        {8,8,0.500000},
        {9,2,0.062500},
        {9,3,0.125000},
        {9,4,0.062500},
        {9,6,0.125000},
        {9,7,0.250000},
        {9,8,0.125000},
        {9,9,0.062500},
        {9,10,0.125000},
        {9,11,0.062500},
        {10,3,0.125000},
        {10,4,0.125000},
        {10,7,0.250000},
        {10,8,0.250000},
        {10,10,0.125000},
        {10,11,0.125000},
        {11,4,0.250000},
        {11,8,0.500000},
        {11,11,0.250000},
        {12,3,0.062500},
        {12,4,0.062500},
        {12,7,0.187500},
        {12,8,0.187500},
        {12,10,0.187500},
        {12,11,0.187500},
        {12,12,0.062500},
        {12,13,0.062500},
        {13,4,0.125000},
        {13,8,0.375000},
        {13,11,0.375000},
        {13,13,0.125000},
        {14,4,0.062500},
        {14,8,0.250000},
        {14,11,0.375000},
        {14,13,0.250000},
        {14,14,0.062500}
    };

static IndexedWeights order5TopWeights [] =
    {
        {0,0,0.062500},
        {0,5,0.250000},
        {0,9,0.375000},
        {0,12,0.250000},
        {0,14,0.062500},
        {1,1,0.062500},
        {1,5,0.062500},
        {1,6,0.187500},
        {1,9,0.187500},
        {1,10,0.187500},
        {1,12,0.187500},
        {1,13,0.062500},
        {1,14,0.062500},
        {2,2,0.062500},
        {2,6,0.125000},
        {2,7,0.125000},
        {2,9,0.062500},
        {2,10,0.250000},
        {2,11,0.062500},
        {2,12,0.125000},
        {2,13,0.125000},
        {2,14,0.062500},
        {3,3,0.062500},
        {3,7,0.187500},
        {3,8,0.062500},
        {3,10,0.187500},
        {3,11,0.187500},
        {3,12,0.062500},
        {3,13,0.187500},
        {3,14,0.062500},
        {4,4,0.062500},
        {4,8,0.250000},
        {4,11,0.375000},
        {4,13,0.250000},
        {4,14,0.062500},
        {5,5,0.125000},
        {5,9,0.375000},
        {5,12,0.375000},
        {5,14,0.125000},
        {6,6,0.125000},
        {6,9,0.125000},
        {6,10,0.250000},
        {6,12,0.250000},
        {6,13,0.125000},
        {6,14,0.125000},
        {7,7,0.125000},
        {7,10,0.250000},
        {7,11,0.125000},
        {7,12,0.125000},
        {7,13,0.250000},
        {7,14,0.125000},
        {8,8,0.125000},
        {8,11,0.375000},
        {8,13,0.375000},
        {8,14,0.125000},
        {9,9,0.250000},
        {9,12,0.500000},
        {9,14,0.250000},
        {10,10,0.250000},
        {10,12,0.250000},
        {10,13,0.250000},
        {10,14,0.250000},
        {11,11,0.250000},
        {11,13,0.500000},
        {11,14,0.250000},
        {12,12,0.500000},
        {12,14,0.500000},
        {13,13,0.500000},
        {13,14,0.500000},
        {14,14,1.000000}
    };

static IndexedWeights order5CenterWeights [] =
    {
        {0,4,0.062500},
        {0,8,0.250000},
        {0,11,0.375000},
        {0,13,0.250000},
        {0,14,0.062500},
        {1,3,0.062500},
        {1,7,0.187500},
        {1,8,0.062500},
        {1,10,0.187500},
        {1,11,0.187500},
        {1,12,0.062500},
        {1,13,0.187500},
        {1,14,0.062500},
        {2,2,0.062500},
        {2,6,0.125000},
        {2,7,0.125000},
        {2,9,0.062500},
        {2,10,0.250000},
        {2,11,0.062500},
        {2,12,0.125000},
        {2,13,0.125000},
        {2,14,0.062500},
        {3,1,0.062500},
        {3,5,0.062500},
        {3,6,0.187500},
        {3,9,0.187500},
        {3,10,0.187500},
        {3,12,0.187500},
        {3,13,0.062500},
        {3,14,0.062500},
        {4,0,0.062500},
        {4,5,0.250000},
        {4,9,0.375000},
        {4,12,0.250000},
        {4,14,0.062500},
        {5,3,0.062500},
        {5,4,0.062500},
        {5,7,0.187500},
        {5,8,0.187500},
        {5,10,0.187500},
        {5,11,0.187500},
        {5,12,0.062500},
        {5,13,0.062500},
        {6,2,0.062500},
        {6,3,0.062500},
        {6,6,0.125000},
        {6,7,0.187500},
        {6,8,0.062500},
        {6,9,0.062500},
        {6,10,0.187500},
        {6,11,0.125000},
        {6,12,0.062500},
        {6,13,0.062500},
        {7,1,0.062500},
        {7,2,0.062500},
        {7,5,0.062500},
        {7,6,0.187500},
        {7,7,0.125000},
        {7,9,0.125000},
        {7,10,0.187500},
        {7,11,0.062500},
        {7,12,0.062500},
        {7,13,0.062500},
        {8,0,0.062500},
        {8,1,0.062500},
        {8,5,0.187500},
        {8,6,0.187500},
        {8,9,0.187500},
        {8,10,0.187500},
        {8,12,0.062500},
        {8,13,0.062500},
        {9,2,0.062500},
        {9,3,0.125000},
        {9,4,0.062500},
        {9,6,0.125000},
        {9,7,0.250000},
        {9,8,0.125000},
        {9,9,0.062500},
        {9,10,0.125000},
        {9,11,0.062500},
        {10,1,0.062500},
        {10,2,0.125000},
        {10,3,0.062500},
        {10,5,0.062500},
        {10,6,0.187500},
        {10,7,0.187500},
        {10,8,0.062500},
        {10,9,0.062500},
        {10,10,0.125000},
        {10,11,0.062500},
        {11,0,0.062500},
        {11,1,0.125000},
        {11,2,0.062500},
        {11,5,0.125000},
        {11,6,0.250000},
        {11,7,0.125000},
        {11,9,0.062500},
        {11,10,0.125000},
        {11,11,0.062500},
        {12,1,0.062500},
        {12,2,0.187500},
        {12,3,0.187500},
        {12,4,0.062500},
        {12,5,0.062500},
        {12,6,0.187500},
        {12,7,0.187500},
        {12,8,0.062500},
        {13,0,0.062500},
        {13,1,0.187500},
        {13,2,0.187500},
        {13,3,0.062500},
        {13,5,0.062500},
        {13,6,0.187500},
        {13,7,0.187500},
        {13,8,0.062500},
        {14,0,0.062500},
        {14,1,0.250000},
        {14,2,0.375000},
        {14,3,0.250000},
        {14,4,0.062500}
    };

double TrinomialCoefficientCalc (int n, int i, int j, int k)
	{
	return s_fact[n]/(s_fact[i]*s_fact[j]*s_fact[k]);
	}

double TrinomialCoefficientCalcC (int n, int i, int j, int k)
	{
	int m = n+1;
	if (i<m && i>-1 && j<m && j>-1 && k<m && k>-1)
		{
		return s_fact[n]/(s_fact[i]*s_fact[j]*s_fact[k]);
		}
	else
		return 0;	
	}

double TrinomialCoefficient (int n, int i, int j, int k)
	{
	return s_trinomial[n][i][j];
	}

double TrinomialCoefficientC (int n, int i, int j, int k)
	{
	int m = n+1;
	if (i<m && i>-1 && j<m && j>-1 && k<m && k>-1)
		{
		return s_trinomial[n][i][j];
		}
	else
		return 0;
	}

// Index this array by [triangleOrder][edgeIndex][pointIndexAlongEdge]
// for edge index = (0:bottom), (1:right), (2:left)
// This gives the control point indices for walking each edge counterclockwise
static int s_edgePoints [6][3][5] =
  {
    // order 0 -- just placeholders for no points !!!
    {
      {-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1}
    },
    // order 1
    {
      {0,-1,-1,-1,-1},
      {0,-1,-1,-1,-1},
      {0,-1,-1,-1,-1}
    },
    // order 2 -- linear triangle
    {
      {0,1,-1,-1,-1},
      {1,2,-1,-1,-1},
      {2,0,-1,-1,-1}
    },
    // order 3 -- quadratric
    {
      {0,1,2,-1,-1},
      {2,4,5,-1,-1},
      {5,3,0,-1,-1}
    },
    // order 4 -- cubic
    {
      {0,1,2,3,-1},
      {3,6,8,9,-1},
      {9,7,4,0,-1}
    },
    // order 5 -- quartic
    {
      {0,1,2,3,4},
      {4,8,11,13,14},
      {14,12,9,5,0},
    },
  };

static int s_numPoints [MAX_BEZIER_TRIANGLE_ORDER + 1] = {0, 1, 3, 6, 10, 15};
int BezierTriangleDPoint3d::GetOrder () const{ return m_order;}
int BezierTriangleDPoint3d::GetDegree () const { return m_order - 1;}

int BezierTriangleDPoint3d::GetNumberPoints () const
    {
    if (m_order == 0 || m_order > MAX_BEZIER_TRIANGLE_ORDER)
        return 0;
    return s_numPoints[m_order];
    }

BezierTriangleDPoint3d::BezierTriangleDPoint3d ()
        : m_order (0)
    {
    }

double BezierTriangleDPoint3d::ControlPolygonArea () const
	{
	DVec3d normalVector;
	double area = 0.0;
	double totalArea = 0.0;

	int ind0, ind1, ind2;

	ind0 =
		ind1 =
			ind2 = 0;

	int n = m_order;
	int m = n-1;
	int i0=0;
	int i1=n;

	for (;m>0;i0=i1,i1+=m,m--)
		{
		for (int i=0;i<m;i++)
			{
			normalVector.CrossProductToPoints (m_controlPoints[i0+i], m_controlPoints[i0+i+1], m_controlPoints[i1+i]);
			area = 0.5 * normalVector.Magnitude ();
			totalArea += area;
			}
		}

	m = n-1;
	i0=0;
	i1=n;

	for (;m>1;i0=i1,i1+=m,m--)
		{
		for (int i=0;i<m-1;i++)
			{
			normalVector.CrossProductToPoints (m_controlPoints[i0+i+1], m_controlPoints[i1+i], m_controlPoints[i1+i+1]);
			area = 0.5 * normalVector.Magnitude ();
			totalArea += area;
			}
		}

	return totalArea;
	}

BezierTriangleDPoint3d::BezierTriangleDPoint3d (DPoint3dCR xyz00, DPoint3dCR xyz10, DPoint3dCR xyz01)
: m_order (2)
    {
    m_controlPoints[0] = xyz00;
    m_controlPoints[1] = xyz10;
    m_controlPoints[2] = xyz01;
    }

BezierTriangleDPoint3d::BezierTriangleDPoint3d (int order)
	: m_order (order)
	{
	}

BezierTriangleDPoint3d::BezierTriangleDPoint3d (DPoint3d *points, int size)
	{
	for (int i=0; i<size; i++)
		{
		m_controlPoints[i] = points[i];
		}
    int order = 0;
    for (; order + 1 <= MAX_BEZIER_TRIANGLE_ORDER && size >= s_numPoints[order + 1];)
        {
        order++;
        }
    m_order = order;
	}

BezierTriangleDPoint3d::BezierTriangleDPoint3d (DVec3d *points, int size)
	{
	for (int i=0; i<size; i++)
		{
		m_controlPoints[i].x = points[i].x;
		m_controlPoints[i].y = points[i].y;
		m_controlPoints[i].z = points[i].z;
		}
    int order = 0;
    for (; order + 1 <= MAX_BEZIER_TRIANGLE_ORDER && size >= s_numPoints[order + 1];)
        {
        order++;
        }
    m_order = order;
	}

int BezierTriangleDPoint3d::SetControlPoints (bvector<DPoint3d> const &controlPoints)
    {
    int order = 0;
    int totalPoints = (int) controlPoints.size ();
    for (; order + 1 <= MAX_BEZIER_TRIANGLE_ORDER && totalPoints >= s_numPoints[order + 1];)
        {
        order++;
        }
    m_order = order;
    for (int numPoints = s_numPoints[order], i = 0; i < numPoints; i++)
        m_controlPoints[i] = controlPoints[i];
    return order;
    }

BezierTriangleDPoint3d::BezierTriangleDPoint3d (bvector<DPoint3d> const &controlPoints)
    {
    SetControlPoints (controlPoints);
    }

DPoint3d BezierTriangleDPoint3d::GetPole (int i) const
	{
	return m_controlPoints[i];
	}

bool BezierTriangleDPoint3d::TryGetPole (int i, int j, DPoint3dR xyz) const
	{
	int n = m_order;

	if (j<n && i<n-j && j>-1 && i>-1)
		{
		int pn = i;
		for (;j>0;j--)
			{
			pn += n;
			n--;
			}

		xyz = GetPole (pn);
		return true;
		} 
	else
		{
		return false;
		}
	}

void BezierTriangleDPoint3d::SetPole (int i, DPoint3dCR xyz)
	{
	m_controlPoints[i] = xyz;
	}

void BezierTriangleDPoint3d::AddToPole (int i, DPoint3dCR xyz, double scale)
	{
	m_controlPoints[i].x += xyz.x * scale;
	m_controlPoints[i].y += xyz.y * scale;
	m_controlPoints[i].z += xyz.z * scale;
	}

void BezierTriangleDPoint3d::ScalePole (int i, double scale)
	{
	DPoint3d xyz;
	xyz.Zero ();
	xyz.x = m_controlPoints[i].x * scale;
	xyz.y = m_controlPoints[i].y * scale;
	xyz.z = m_controlPoints[i].z * scale;
	SetPole (i, xyz);
	}

void BezierTriangleDPoint3d::SetPole (int i, int j, DPoint3dCR xyz)
	{
	int n = m_order;
	int pn = i;

	for (;j>0;j--)
		{
		pn += n;
		n--;
		}

	m_controlPoints[pn] = xyz;
	}

void BezierTriangleDPoint3d::SetPole (int i, int j, DVec3dCR xyz)
	{
	int n = m_order;
	int pn = i;

	for (;j>0;j--)
		{
		pn += n;
		n--;
		}

	m_controlPoints[pn].x = xyz.x;
	m_controlPoints[pn].y = xyz.y;
	m_controlPoints[pn].z = xyz.z;
	}

void BezierTriangleDPoint3d::Zero ()
	{
	int np = (m_order*(m_order+1))/2;
	DPoint3d orig = DPoint3d::From (0,0,0);
	for (int i=0; i<np; i++)
		{
		m_controlPoints[i] = orig;
		}
	}

BezierTriangleDPoint3d BezierTriangleDPoint3d::RaiseOrder () const
	{
	DPoint3d p [3];
	DPoint3d pnt, orig;
	orig = DPoint3d::From (0,0,0);
	BezierTriangleDPoint3d newtriangle;
	int ord = m_order;
	newtriangle.m_order = ord+1;

	for (int j=0; j<ord+1; j++)
		{
		for (int i=0; i<ord+1-j; i++)
			{
			TryGetPole (i-1, j, p[0]);
			TryGetPole (i, j-1, p[1]);
			TryGetPole (i,j, p[2]);

			pnt = DPoint3d::FromSumOf (orig, p[0], ((double)i)/ord, p[1], ((double)j)/ord, p[2], (ord-(double)i-(double)j)/ord);
			newtriangle.SetPole (i, j, pnt);
			}
		}

	return newtriangle;
	}

BezierTriangleDPoint3d BezierTriangleDPoint3d::RaiseOrderTo (int order) const
	{
	BezierTriangleDPoint3d newtriangle[6];
	newtriangle[m_order].m_order = m_order;
	int np = (m_order*(m_order+1))/2;

	for (int i=0; i<np; i++)
		{
		newtriangle[m_order].m_controlPoints[i] = this->m_controlPoints[i];
		}

	for (int j=m_order; j<order; j++)
		{
		newtriangle[j+1].m_order = j+1;
		newtriangle[j+1] = newtriangle[j].RaiseOrder ();
		}

	return newtriangle[order];
	}

void BezierTriangleDPoint3d::Multiply (TransformCR transform)
	{
	int np = (m_order * (m_order+1))/2;
	transform.Multiply (m_controlPoints, np);
	}

BezierTriangleDPoint3d BezierTriangleDPoint3d::FromMultiply (TransformCR transform, BezierTriangleDPoint3dCR triangle)
	{
	BezierTriangleDPoint3d newTriangle = triangle;
	newTriangle.Multiply (transform);
	return newTriangle;
	}

DPoint3d BezierTriangleDPoint3d::EvaluateDirect (DPoint3dCR uvw) const
	{
	if (m_order>0)
		{
		int np = (m_order*(m_order+1))/2;
		double basfunc[15];
		ComputeBasisFunctions (uvw.x, uvw.y, basfunc);
		return AddScaled (m_controlPoints, basfunc, np);
		}
	else
		{
		return DPoint3d::From (0,0,0);
		}
	}

DPoint3d BezierTriangleDPoint3d::EvaluateDirect (double u, double v) const
	{
	if (m_order>0)
		{
		int np = (m_order*(m_order+1))/2;
		double basfunc[15];
		ComputeBasisFunctions (u, v, basfunc);
		return AddScaled (m_controlPoints, basfunc, np);
		}
	else
		{
		return DPoint3d::From (0,0,0);
		}
	}

void BezierTriangleDPoint3d::EvaluateDirect (double u, double v, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) const
	{
	if (m_order>0)
		{
		int np = (m_order*(m_order+1))/2;
		double basfunc[15], basfuncu[15], basfuncv[15];
		ComputeBasisFunctions (u, v, basfunc, basfuncu, basfuncv);
		xyz = AddScaled (m_controlPoints, basfunc, np);
		dXdu = DVec3d::From (AddScaled (m_controlPoints, basfuncu, np));
		dXdv = DVec3d::From (AddScaled (m_controlPoints, basfuncv, np));
		}
	else
		{
		xyz = DPoint3d::From (0,0,0);
		dXdu = DVec3d::FromStartEnd (DPoint3d::From (0,0,0), DPoint3d::From (0,0,0));
		dXdv = DVec3d::FromStartEnd (DPoint3d::From (0,0,0), DPoint3d::From (0,0,0));
		}
	}

void BezierTriangleDPoint3d::ComputeBasisFunctions (double u, double v, double *values) const
	{
	if (m_order == 2)
		{
		values[0] = 1.0 - u - v;
		values[1] = u;
		values[2] = v;
		}
	else if (m_order == 3)
		{
		double w = 1.0 - u - v;
		values[0] = w*w;
		values[1] = 2*u*w;
		values[2] = u*u; 
		values[3] = 2*v*w;
		values[4] = 2*u*v;
		values[5] = v*v;
		}
	else if (m_order == 4)
		{
		double w = 1.0 - u - v;
		double uu = u*u;
		double vv = v*v;
		double ww = w*w;
		double thrus = 3*uu;
		double thrvs = 3*vv;
		double thrws = 3*ww;

		values[0] = ww*w;
		values[1] = thrws*u;
		values[2] = thrus*w;
		values[3] = uu*u;
		values[4] = thrws*v;
		values[5] = 6*u*v*w;
		values[6] = thrus*v;
		values[7] = thrvs*w;
		values[8] = thrvs*u;
		values[9] = vv*v;
		}
	else if (m_order == 5)
		{
		double w = 1.0 - u - v;
		double uu = u*u;
		double vv = v*v;
		double ww = w*w;
		double uuu = uu*u;
		double vvv = vv*v;
		double www = ww*w;
		double fuw = 4*u*w;
		double fuv = 4*u*v;
		double fvw = 4*v*w;
		double tuvw = fuw*3*v;

		values[0] = www*w;
		values[1] = fuw*ww;
		values[2] = 6*uu*ww;
		values[3] = fuw*uu;
		values[4] = uuu*u;
		values[5] = fvw*ww;
		values[6] = tuvw*w;
		values[7] = tuvw*u;
		values[8] = fuv*uu;
		values[9] = 6*ww*vv;
		values[10] = tuvw*v;
		values[11] = 6*uu*vv;
		values[12] = fvw*vv;
		values[13] = fuv*vv;
		values[14] = vvv*v;
		}
	}

void BezierTriangleDPoint3d::ComputeBasisFunctions (double u, double v, double *values, double *ddu, double *ddv) const
	{
	if (m_order == 2)
		{
		values[0] = 1.0 - u - v;
		values[1] = u;
		values[2] = v;

		ddu[0] = -1;
		ddu[1] = 1;
		ddu[2] = 0;

		ddv[0] = -1;
		ddv[1] = 0;
		ddv[2] = 1;
		}
	else if (m_order == 3)
		{
		double w = 1.0 - u - v;

		values[0] = w*w;
		values[1] = 2*u*w;
		values[2] = u*u; 
		values[3] = 2*v*w;
		values[4] = 2*u*v;
		values[5] = v*v;

		ddu[0] = -2*w;
		ddu[1] = 2*w - 2*u;
		ddu[2] = 2*u;
		ddu[3] = -2*v;
		ddu[4] = 2*v;
		ddu[5] = 0;

		ddv[0] = -2*w;
		ddv[1] = -2*u;
		ddv[2] = 0;
		ddv[3] = 2*w - 2*v;
		ddv[4] = 2*u;
		ddv[5] = 2*v;
		}
	else if (m_order == 4)
		{
		double w = 1.0 - u - v;
		double uu = u*u;
		double vv = v*v;
		double ww = w*w;
		double thrus = 3*uu;
		double thrvs = 3*vv;
		double thrws = 3*ww;
		double sxuw = 6*u*w;
		double sxuv = 6*u*v;
		double sxvw = 6*v*w;

		values[0] = ww*w;
		values[1] = thrws*u;
		values[2] = thrus*w;
		values[3] = uu*u;
		values[4] = thrws*v;
		values[5] = 6*u*v*w;
		values[6] = thrus*v;
		values[7] = thrvs*w;
		values[8] = thrvs*u;
		values[9] = vv*v;

		ddu[0] = -thrws;
		ddu[1] = thrws - sxuw;
		ddu[2] = sxuw - thrus;
		ddu[3] = thrus;
		ddu[4] = -sxvw;
		ddu[5] = sxvw - sxuv;
		ddu[6] = sxuv;
		ddu[7] = -thrvs;
		ddu[8] = thrvs;
		ddu[9] = 0;

		ddv[0] = -thrws;
		ddv[1] = -sxuw;
		ddv[2] = -thrus;
		ddv[3] = 0;
		ddv[4] = thrws - sxvw;
		ddv[5] = sxuw - sxuv;
		ddv[6] = thrus;
		ddv[7] = sxvw - thrvs;
		ddv[8] = sxuv;
		ddv[9] = thrvs;
		}
	else if (m_order == 5)
		{
		double w = 1.0 - u - v;
		double uu = u*u;
		double vv = v*v;
		double ww = w*w;
		double uuu = uu*u;
		double fuuu = 4*uuu;
		double vvv = vv*v;
		double fvvv = 4*vvv;
		double www = ww*w;
		double fwww = 4*www;
		double fuw = 4*u*w;
		double tuws = fuw*3*w;
		double tusw = fuw*3*u;
		double fuv = 4*u*v;
		double tuvs = fuv*3*v;
		double tusv = fuv*3*u;
		double fvw = 4*v*w;
		double tvws = fvw*3*w;
		double tvsw = fvw*3*v;
		double tuvw = fuw*3*v;
		double twfuvw = 2*tuvw;


		values[0] = www*w;
		values[1] = fuw*ww;
		values[2] = 6*uu*ww;
		values[3] = fuw*uu;
		values[4] = uuu*u;
		values[5] = fvw*ww;
		values[6] = tuvw*w;
		values[7] = tuvw*u;
		values[8] = fuv*uu;
		values[9] = 6*ww*vv;
		values[10] = tuvw*v;
		values[11] = 6*uu*vv;
		values[12] = fvw*vv;
		values[13] = fuv*vv;
		values[14] = vvv*v;

		ddu[0] = -fwww;
		ddu[1] = fwww - tuws;
		ddu[2] = tuws -  tusw;
		ddu[3] = tusw - fuuu;
		ddu[4] = fuuu;
		ddu[5] = -tvws;
		ddu[6] = tvws - twfuvw;
		ddu[7] = twfuvw - tusv;
		ddu[8] = tusv;
		ddu[9] = -tvsw;
		ddu[10] = tvsw - tuvs;
		ddu[11] = tuvs;
		ddu[12] = -fvvv;
		ddu[13] = fvvv;
		ddu[14] = 0;

		ddv[0] = -fwww;
		ddv[1] = -tuws;
		ddv[2] = -tusw;
		ddv[3] = -fuuu;
		ddv[4] = 0;
		ddv[5] = fwww - tvws;
		ddv[6] = tuws - twfuvw;
		ddv[7] = tusw - tusv;
		ddv[8] = fuuu;
		ddv[9] = tvws - tvsw;
		ddv[10] = twfuvw - tuvs;
		ddv[11] = tusv;
		ddv[12] = tvsw - fvvv;
		ddv[13] = tuvs;
		ddv[14] = fvvv;
		}
	}

DPoint3d BezierTriangleDPoint3d::EvaluateDirectCompact (double u, double v) const
	{
	if (m_order>0)
		{
		int np = (m_order*(m_order+1))/2;
		double basfunc[15];
		ComputeBasisFunctionsCompact (u, v, basfunc);
		return AddScaled (m_controlPoints, basfunc, np);
		}
	else
		{
		return DPoint3d::From (0,0,0);
		}
	
	}

void BezierTriangleDPoint3d::EvaluateDirectCompact (double u, double v, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) const
	{
	if (m_order>0)
		{
		int np = (m_order*(m_order+1))/2;
		double basfunc[15], basfuncu[15], basfuncv[15];
		ComputeBasisFunctionsCompact (u, v, basfunc, basfuncu, basfuncv);
		xyz = AddScaled (m_controlPoints, basfunc, np);
		dXdu = DVec3d::From (AddScaled (m_controlPoints, basfuncu, np));
		dXdv = DVec3d::From (AddScaled (m_controlPoints, basfuncv, np));
		}
	else
		{
		xyz = DPoint3d::From (0,0,0);
		dXdu = DVec3d::FromStartEnd (DPoint3d::From (0,0,0), DPoint3d::From (0,0,0));
		dXdv = DVec3d::FromStartEnd (DPoint3d::From (0,0,0), DPoint3d::From (0,0,0));
		}
	
	}

//! direction=0 when in direction u, and 1 when in direction of v

BezierTriangleDPoint3d BezierTriangleDPoint3d::EvaluateDirectionalDifference (int direction)
	{
	//int n = m_order-2;
	if (m_order>0)
		{
		int m = m_order-1;
		int mpi = (m_order*(m_order+1))/2 - 1;
		int mpf = (m*(m+1))/2 - 1;
		int ind = mpi - 2;
		int q=0;
		int trk = 3;
		int vtr = 2;
		BezierTriangleDPoint3d triangle;
		triangle.m_order = m_order - 1;

		for (;m>0;m--)
			{
			for (int j=0;j<m_order-m;j++)
				{
				if (direction = 0)
					{
					triangle.m_controlPoints[mpf-q] = DVec3d::FromStartEnd (m_controlPoints[ind-j], m_controlPoints[ind+1-j]);
					}
				else if (direction = 1 )
					{
					triangle.m_controlPoints[mpf-q] = DVec3d::FromStartEnd (m_controlPoints[ind-j], m_controlPoints[ind+vtr-j]);
					}
				q++;
				}
			ind += 1 - trk;
			trk++;
			vtr++;
			}

		return triangle;
		}
	else
		{
		return BezierTriangleDPoint3d (0);
		}

	/*m=n;
	q=0;

	for (;m>0;m--)
		{
		for (int j=0;j<m;j++)
			{
			tr[mpf-q].scale((m_order-1)*s_trinomial[n][m][j]);
			q++;
			}
		}*/
	}

void BezierTriangleDPoint3d::ComputeBasisFunctionsCompact (double u, double v, double *values) const
        { 
		if (m_order>0)
			{
			double w = 1.0 - u - v; 
			int n = m_order - 1;
			int m = n; 
			int q = 0;     
			int mp = (m_order*(m_order+1))/2 - 1; 
			double valPower [3][5];
	
			for (int w=0; w<3; w++)
				{
				valPower[w][0] = 1;
				}

			for (int p=0; p<m; p++)
				{
				valPower[0][p+1]= valPower[0][p]*u;
				valPower[1][p+1]= valPower[1][p]*v;
				valPower[2][p+1]= valPower[2][p]*w;
				}

			for (;m>-1; m--) 
				{ 
				for (int j=0; j<m_order-m; j++) 
					{ 
					int k = n - m - j; 
					//values[mp-q] = TrinomialCoefficient(n,m,j,k)*valPower[1][m]*valPower[2][j]*valPower[0][k]; 
					values[mp-q] = s_trinomial[n][m][j]*valPower[1][m]*valPower[2][j]*valPower[0][k];
					q++; 
					} 
				} 
				}
			else
				{
				}
        }

void BezierTriangleDPoint3d::ComputeBasisFunctionsCompact (double u, double v, double *values, double *ddu, double *ddv) const
        {
		if (m_order>0)
			{
			double w = 1.0 - u - v; 
			int n = m_order - 1;
			int m = n; 
			int q = 1;     
			int np = (m_order*(m_order+1))/2; 
			double valPower [3][5];
	
			for (int w=0; w<3; w++)
				{
				valPower[w][0] = 1;
				}

			for (int p=0; p<m; p++)
				{
				valPower[0][p+1]= valPower[0][p]*u;
				valPower[1][p+1]= valPower[1][p]*v;
				valPower[2][p+1]= valPower[2][p]*w;
				}

			for (;m>-1; m--) 
				{ 
				for (int j=0; j<m_order - m; j++) 
					{ 
					int k = n - m - j; 
					//values[np-q] = TrinomialCoefficient(n,m,j,k)*valPower[1][m]*valPower[2][j]*valPower[0][k]; 
					values[np-q] = s_trinomial[n][m][j]*valPower[1][m]*valPower[2][j]*valPower[0][k];
					ddu[np-q] = n*(TrinomialCoefficientC(n-1,m,j,k-1)*valPower[1][m]*valPower[2][j]*valPower[0][k-1] -  TrinomialCoefficientC(n-1,m,j-1,k)*valPower[1][m]*valPower[2][j-1]*valPower[0][k]);
					ddv[np-q] = n*(TrinomialCoefficientC(n-1,m-1,j,k)*valPower[1][m-1]*valPower[2][j]*valPower[0][k] -  TrinomialCoefficientC(n-1,m,j-1,k)*valPower[1][m]*valPower[2][j-1]*valPower[0][k]);
					q++; 
					} 
				} 
			} 
		else
			{
			}
        }

void BezierTriangleDPoint3d::InplaceDeCasteljauLayer (double u, double v)
	{
	DPoint3d points[3];
	double scales [3];
	scales[0] = 1.0 - u - v;
	scales[1] = u;
	scales[2] = v;
	int m = m_order -1;
	int i0 = 0;
	int i1 = m_order;
	int k = 0;
	for (;m > 0;i0=i1,i1+=m,m--)
		{		
		for (int j=0; j < m; j++)
			{
			points[0] = m_controlPoints[i0+j];
			points[1] = m_controlPoints[i0+j+1];
			points[2] = m_controlPoints[i1+j];
			m_controlPoints[k] = AddScaled (points, scales, 3);
			k++;
			}
		}
	m_order--;
	}

void BezierTriangleDPoint3d::LayerTriangles (double u, double v, BezierTriangleDPoint3dR bottom, BezierTriangleDPoint3dR right, BezierTriangleDPoint3dR left)
	{
	int n = m_order;
	int k = 0;

	bottom.m_order = 
		right.m_order = 
			left.m_order = n;

	BezierTriangleDPoint3d workTriangle = *this;

	for (int q=n; q>1; q--)
		{
		for (int j=0; j<q; j++)
			{
			bottom.m_controlPoints[k] = workTriangle.m_controlPoints[s_edgePoints[q][0][j]];
			right.m_controlPoints[k] = workTriangle.m_controlPoints[s_edgePoints[q][1][j]];
			left.m_controlPoints[k] = workTriangle.m_controlPoints[s_edgePoints[q][2][j]];
			k++;
			}		
		workTriangle.InplaceDeCasteljauLayer (u, v);
		}

	bottom.m_controlPoints[k] = 
		right.m_controlPoints[k] = 
			left.m_controlPoints[k] = workTriangle.m_controlPoints[0];
	}

DPoint3d BezierTriangleDPoint3d::LayerTrianglesEvaluate (double u, double v) const
	{
	double scales [3];
	DPoint3d points[3];

	BezierTriangleDPoint3d workTriangle = *this;

	for (int q=m_order; q>2; q--)
		{
		workTriangle.InplaceDeCasteljauLayer (u, v);
		}

	scales[0] = 1.0 - u - v;
	scales[1] = u;
	scales[2] = v;
	
	for (int i=0; i<3; i++)
		{
		points[i] = workTriangle.m_controlPoints[i];
		}

	return AddScaled (points, scales, 3);
	}

void BezierTriangleDPoint3d::LayerTrianglesEvaluate (double u, double v, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) const
	{
	double sc = m_order - 1;
	double scales [3];
	DPoint3d points[3];

	BezierTriangleDPoint3d workTriangle = *this;

	for (int q=m_order; q>2; q--)
		{
		workTriangle.InplaceDeCasteljauLayer (u, v);
		}

	scales[0] = 1.0 - u - v;
	scales[1] = u;
	scales[2] = v;
	
	for (int i=0; i<3; i++)
		{
		points[i] = workTriangle.m_controlPoints[i];
		}

	xyz = AddScaled (points, scales, 3);
	dXdu = DVec3d::FromStartEnd (points[0], points[1]);
	dXdv = DVec3d::FromStartEnd (points[0], points[2]);

	dXdu.scale(sc);
	dXdv.scale(sc);
	}

void BezierTriangleDPoint3d::NoOp (double u, double v)
	{
	}

void BezierTriangleDPoint3d::SymmetricSplit (BezierTriangleDPoint3dR triangle00, BezierTriangleDPoint3dR triangle10, BezierTriangleDPoint3dR triangle01, BezierTriangleDPoint3dR trianglecenter)
	{
	BezierTriangleDPoint3d bottom, right, left, bottoml, rightl, leftl, bottomr, rightr, leftr, bottomrr, rightrr, leftrr;
	LayerTriangles (0.5, 0.0, bottom, right, left);
	left.LayerTriangles (0.5, 0.0, bottoml, triangle00, leftl);
	right.LayerTriangles (0.5, 0.0, bottomr, rightr, triangle10);
	rightr.LayerTriangles (1.0, -1.0, bottomrr, trianglecenter, triangle01);
	}

void BezierTriangleDPoint3d::ComputeWeighedChildTriangle (BezierTriangleDPoint3dR child, IndexedWeights *weights, int numWeights)
      {
      child.Zero ();
      for (int i = 0; i < numWeights; i++)
             {
             int childIndex = weights[i].childIndex;
             int parentIndex = weights[i].parentIndex;
			 double w = weights[i].weight;
#ifdef expandAddToPole
			 //This is about 5% faster
			 child.m_controlPoints[childIndex].x += m_controlPoints[parentIndex].x * w;
			 child.m_controlPoints[childIndex].y += m_controlPoints[parentIndex].y * w;
			 child.m_controlPoints[childIndex].z += m_controlPoints[parentIndex].z * w;
#else 
			 child.AddToPole(childIndex, m_controlPoints[parentIndex],w);
#endif
             }
		}

void BezierTriangleDPoint3d::SymmetricSplitDirect (BezierTriangleDPoint3dR triangle00, BezierTriangleDPoint3dR triangle10, BezierTriangleDPoint3dR triangle01, BezierTriangleDPoint3dR trianglecenter)
	{
	triangle00.m_order =
		triangle01.m_order =
			triangle10.m_order =
				trianglecenter.m_order = GetOrder ();

	if (m_order == 2)
		{
		ComputeWeighedChildTriangle (triangle00, order2LeftWeights, 5);
		ComputeWeighedChildTriangle (triangle10, order2RightWeights, 5);
		ComputeWeighedChildTriangle (triangle01, order2TopWeights, 5);
		ComputeWeighedChildTriangle (trianglecenter, order2CenterWeights, 6);
		}
	else if (m_order == 3)
		{
		ComputeWeighedChildTriangle (triangle00, order3LeftWeights, 15);
		ComputeWeighedChildTriangle (triangle10, order3RightWeights, 15);
		ComputeWeighedChildTriangle (triangle01, order3TopWeights, 15);
		ComputeWeighedChildTriangle (trianglecenter, order3CenterWeights, 21);
		}
	else if (m_order == 4)
		{
		ComputeWeighedChildTriangle (triangle00, order4LeftWeights, 35);
		ComputeWeighedChildTriangle (triangle10, order4RightWeights, 35);
		ComputeWeighedChildTriangle (triangle01, order4TopWeights, 35);
		ComputeWeighedChildTriangle (trianglecenter, order4CenterWeights, 55);
		}
	else if (m_order == 5)
		{
		ComputeWeighedChildTriangle (triangle00, order5LeftWeights, 70);
		ComputeWeighedChildTriangle (triangle10, order5RightWeights, 70);
		ComputeWeighedChildTriangle (triangle01, order5TopWeights, 70);
		ComputeWeighedChildTriangle (trianglecenter, order5CenterWeights, 120);
		}
	}










BezierTriangleD::BezierTriangleD ()
    {
    }

BezierTriangleD::BezierTriangleD (double *points, int size)
	{
	for (int i=0; i<size; i++)
		{
		m_controlPoints[i] = points[i];
		}
    int order = 0;
    for (; order + 1 <= MAX_BEZIER_TRIANGLE_ORDER && size >= s_numPoints[order + 1];)
        {
        order++;
        }
    m_order = order;
	}

void BezierTriangleD::Zero (int np)
	{
	for (int i=0; i<np; i++)
		{
		m_controlPoints[i] = 0;
		}
	}

void BezierTriangleD::AddToPole (int i, double val)
	{
	m_controlPoints[i] += val;
	}

double BezierTriangleD::GetPole (int i) const
	{
	return m_controlPoints[i];
	}

int BezierTriangleD::GetOrder () const
	{
	return m_order;
	}

void BezierTriangleD::SetOrder (int i)
	{
	m_order = i;
	}

BezierTriangleD AddScaledBez (BezierTriangleDCP points, double *scales, size_t n)
	{
	int m = points[0].GetOrder ();
	int nump = (m*(m+1))/2;
	BezierTriangleD bez;
	bez.Zero (nump);
	for (size_t i = 0; i < n; i++)
		{
		for (int j=0; j<nump; j++)
			{
			bez.AddToPole(j,points[i].GetPole(j) * scales[i]);
			}
		}
	bez.SetOrder(points[0].GetOrder ());
	return bez;	
	}





BezierTriangleOfTriangles::BezierTriangleOfTriangles ()
: m_order (0)
	{
	}

int BezierTriangleOfTriangles::GetOrder () const
	{
	return m_order;
	}

void BezierTriangleOfTriangles::SetPole (int i, BezierTriangleD val)
	{
	m_triangles[i] = val;
	}

BezierTriangleD BezierTriangleOfTriangles::GetPole (int i) const
	{
	return m_triangles[i];
	}

BezierTriangleOfTriangles::BezierTriangleOfTriangles (BezierTriangleD *points, int size)
	{
	for (int i=0; i<size; i++)
		{
		m_triangles[i] = points[i];
		}
    int order = 0;
    for (; order + 1 <= MAX_BEZIER_TRIANGLE_ORDER && size >= s_numPoints[order + 1];)
        {
        order++;
        }
    m_order = order;
	}

void BezierTriangleOfTriangles::InplaceDeCasteljauLayer (double u, double v)
	{
	BezierTriangleD points[3];
	double scales [3];
	scales[0] = 1.0 - u - v;
	scales[1] = u;
	scales[2] = v;
	int m = m_order -1;
	int i0 = 0;
	int i1 = m_order;
	int k = 0;
	for (;m > 0;i0=i1,i1+=m,m--)
		{		
		for (int j=0; j < m; j++)
			{
			points[0] = m_triangles[i0+j];
			points[1] = m_triangles[i0+j+1];
			points[2] = m_triangles[i1+j];
			m_triangles[k] = AddScaledBez (points, scales, 3);
			k++;
			}
		}
	m_order--;
	}



void BezierTriangleOfTriangles::LayerTriangles (double u, double v, BezierTriangleOfTrianglesR bottom, BezierTriangleOfTrianglesR right, BezierTriangleOfTrianglesR left)
	{
	int n = m_order;
	int k = 0;

	bottom.m_order = 
		right.m_order = 
			left.m_order = n;

	BezierTriangleOfTriangles workTriangle = *this;

	for (int q=n; q>1; q--)
		{
		for (int j=0; j<q; j++)
			{
			/*bottom.SetPole(k,workTriangle.m_triangles[s_edgePoints[q][0][j]]);
			right.SetPole(k,workTriangle.m_triangles[s_edgePoints[q][1][j]]);
			left.SetPole(k,workTriangle.m_triangles[s_edgePoints[q][2][j]]);*/

			bottom.m_triangles[k] = workTriangle.m_triangles[s_edgePoints[q][0][j]];
			right.m_triangles[k] = workTriangle.m_triangles[s_edgePoints[q][1][j]];
			left.m_triangles[k] = workTriangle.m_triangles[s_edgePoints[q][2][j]];
			k++;
			}		
		workTriangle.InplaceDeCasteljauLayer (u, v);
		}

	bottom.m_triangles[k] = 
		right.m_triangles[k] = 
			left.m_triangles[k] = workTriangle.m_triangles[0];
	}

void BezierTriangleOfTriangles::SymmetricSplit (BezierTriangleOfTrianglesR triangle00, BezierTriangleOfTrianglesR triangle10, BezierTriangleOfTrianglesR triangle01, BezierTriangleOfTrianglesR trianglecenter)
	{
	BezierTriangleOfTriangles bottom, right, left, bottoml, rightl, leftl, bottomr, rightr, leftr, bottomrr, rightrr, leftrr;
	LayerTriangles (0.5, 0.0, bottom, right, left);
	left.LayerTriangles (0.5, 0.0, bottoml, triangle00, leftl);
	//Set triangle01 = rightr, and rightrr = triangle01!!
	right.LayerTriangles (0.5, 0.0, bottomr, rightr, triangle10);
	rightr.LayerTriangles (1.0, -1.0, bottomrr, trianglecenter, triangle01);
	}



























double BinomialCoefficientSafe (int n, int i)
	{
	if (i>-1 && i<n+1)
		{
		return s_trinomial[n][0][i];
		}
	else
		return 0.0;
	}

static int s_numPoints_crv [MAX_BEZIER_CURVE_ORDER_ForStruct + 1] = {0, 1, 2, 3, 4, 5};
int BezierCurveDPoint3d::GetOrder () const { return m_order;}
int BezierCurveDPoint3d::GetDegree () const { return m_order - 1;}
int BezierCurveDPoint3d::GetNumberPoints () const { return m_order; }

BezierCurveDPoint3d::BezierCurveDPoint3d ()
        : m_order (0)
    {
    }

BezierCurveDPoint3d::BezierCurveDPoint3d (DPoint3dCR xyz0, DPoint3dCR xyz1, DPoint3dCR xyz2)
: m_order (2)
    {
    m_controlPoints[0] = xyz0;
    m_controlPoints[1] = xyz1;
    m_controlPoints[2] = xyz2;
    }

BezierCurveDPoint3d::BezierCurveDPoint3d (int order)
	: m_order (order)
	{
	}

BezierCurveDPoint3d::BezierCurveDPoint3d (DPoint3d *points, int size)
	{
	for (int i=0; i<size; i++)
		{
		m_controlPoints[i] = points[i];
		}
    m_order = size;
	}

BezierCurveDPoint3d::BezierCurveDPoint3d (DVec3d *points, int size)
	{
	for (int i=0; i<size; i++)
		{
		m_controlPoints[i].x = points[i].x;
		m_controlPoints[i].y = points[i].y;
		m_controlPoints[i].z = points[i].z;
		}
    m_order = size;
	}

int BezierCurveDPoint3d::SetControlPoints (bvector<DPoint3d> const &controlPoints)
    {
    int order = (int) controlPoints.size ();
    m_order = order;
    for (int numPoints = s_numPoints_crv[order], i = 0; i < numPoints; i++)
        m_controlPoints[i] = controlPoints[i];
    return order;
    }

BezierCurveDPoint3d::BezierCurveDPoint3d (bvector<DPoint3d> const &controlPoints)
    {
    SetControlPoints (controlPoints);
    }

DPoint3d BezierCurveDPoint3d::GetPole (int i) const
	{
	return m_controlPoints[i];
	}

DPoint3d BezierCurveDPoint3d::GetPoleSafe (int i) const
	{
	if (i>-1 && i<m_order)
		{
		return m_controlPoints[i];
		}
	else
		return DPoint3d::From (0,0,0);
	}

void BezierCurveDPoint3d::SetPole (int i, DPoint3dCR xyz)
	{
	m_controlPoints[i] = xyz;
	}

BezierCurveDPoint3d BezierCurveDPoint3d::RaiseOrder () const
	{
	DPoint3d pnt, orig;
	orig = DPoint3d::From (0,0,0);
	BezierCurveDPoint3d newcurve;
	int ord = m_order;
	newcurve.m_order = ord+1;

	for (int i=0; i<ord+1; i++)
		{
		pnt = DPoint3d::FromSumOf (orig, GetPoleSafe (i-1), ((double)i)/ord, GetPoleSafe (i), (ord-(double)i)/ord);
		newcurve.SetPole (i, pnt);
		}

	return newcurve;
	}

BezierCurveDPoint3d BezierCurveDPoint3d::RaiseOrderTo (int order) const
	{
	BezierCurveDPoint3d newcurve[6];
	newcurve[m_order].m_order = m_order;

	for (int i=0; i<m_order; i++)
		{
		newcurve[m_order].m_controlPoints[i] = this->m_controlPoints[i];
		}

	for (int j=m_order; j<order; j++)
		{
		newcurve[j+1].m_order = j+1;
		newcurve[j+1] = newcurve[j].RaiseOrder ();
		}

	return newcurve[order];
	}

void BezierCurveDPoint3d::Multiply (TransformCR transform)
	{
	transform.Multiply (m_controlPoints, m_order);
	}

BezierCurveDPoint3d BezierCurveDPoint3d::FromMultiply (TransformCR transform, BezierCurveDPoint3dCR curve)
	{
	BezierCurveDPoint3d newCurve = curve;
	newCurve.Multiply (transform);
	return newCurve;
	}

DPoint3d BezierCurveDPoint3d::EvaluateDirect (double u) const
	{
	if (m_order>0)
		{
		double basfunc[5];
		ComputeBasisFunctions (u, basfunc);
		return AddScaled (m_controlPoints, basfunc, m_order);
		}
	else
		{
		return DPoint3d::From (0,0,0);
		}
	}

void BezierCurveDPoint3d::EvaluateDirect (double u, DPoint3dR xyz, DVec3dR dXdu) const
	{
	if (m_order>0)
		{
		double basfunc[5], basfuncu[5];
		ComputeBasisFunctions (u, basfunc, basfuncu);
		xyz = AddScaled (m_controlPoints, basfunc, m_order);
		dXdu = DVec3d::From (AddScaled (m_controlPoints, basfuncu, m_order));
		}
	else
		{
		xyz = DPoint3d::From (0,0,0);
		dXdu = DVec3d::FromStartEnd (DPoint3d::From (0,0,0), DPoint3d::From (0,0,0));
		}
	}

void BezierCurveDPoint3d::ComputeBasisFunctions (double u, double *values) const
	{
	double v = 1.0-u;

	if (m_order == 2)
		{
		values[0] = v;
		values[1] = u;
		}
	else if (m_order == 3)
		{
		values[0] = v*v;
		values[1] = 2*u*v;
		values[2] = u*u; 
		}
	else if (m_order == 4)
		{
		double uu = u*u;
		double vv = v*v;

		values[0] = vv*v;
		values[1] = 3*vv*u;
		values[2] = 3*uu*v;
		values[3] = uu*u;
		}
	else if (m_order == 5)
		{
		double uu = u*u;
		double uuu = uu*u;
		double vv = v*v;
		double vvv = vv*v;

		values[0] = vvv*v;
		values[1] = 4*vvv*u;
		values[2] = 6*uu*vv;
		values[3] = 4*uuu*v;
		values[4] = uuu*u;
		}
	}

void BezierCurveDPoint3d::ComputeBasisFunctions (double u, double *values, double *ddu) const
	{
	double v = 1.0-u;

	if (m_order == 2)
		{
		values[0] = v;
		values[1] = u;

		ddu[0] = -1;
		ddu[1] = 1;
		}
	else if (m_order == 3)
		{
		values[0] = v*v;
		values[1] = 2*u*v;
		values[2] = u*u; 

		ddu[0] = -2*v;
		ddu[1] = 2*v - 2*u;
		ddu[2] = 2*u;
		}
	else if (m_order == 4)
		{
		double uu = u*u;
		double vv = v*v;
		double sxuv = 6*u*v;

		values[0] = vv*v;
		values[1] = 3*vv*u;
		values[2] = 3*uu*v;
		values[3] = uu*u;

		ddu[0] = -3*vv;
		ddu[1] = 3*vv - sxuv;
		ddu[2] = sxuv - 3*uu;
		ddu[3] = 3*uu;
		}
	else if (m_order == 5)
		{
		double uu = u*u;
		double uuu = uu*u;
		double vv = v*v;
		double vvv = vv*v;
		double twuuv = 12*uu*v;
		double twuvv = 12*u*vv;

		values[0] = vvv*v;
		values[1] = 4*vvv*u;
		values[2] = 6*uu*vv;
		values[3] = 4*uuu*v;
		values[4] = uuu*u;

		ddu[0] = -4*vvv;
		ddu[1] = 4*vvv - twuvv;
		ddu[2] = twuvv - twuuv;
		ddu[3] = twuuv - 4*uuu;
		ddu[4] = 4*uuu;
		}
	}

DPoint3d BezierCurveDPoint3d::EvaluateDirectCompact (double u) const
	{
	if (m_order>0)
		{
		double basfunc[5];
		ComputeBasisFunctionsCompact (u, basfunc);
		return AddScaled (m_controlPoints, basfunc, m_order);
		}
	else
		{
		return DPoint3d::From (0,0,0);
		}	
	}

void BezierCurveDPoint3d::EvaluateDirectCompact (double u, DPoint3dR xyz, DVec3dR dXdu) const
	{
	if (m_order>0)
		{
		double basfunc[5], basfuncu[5];
		ComputeBasisFunctionsCompact (u, basfunc, basfuncu);
		xyz = AddScaled (m_controlPoints, basfunc, m_order);
		dXdu = DVec3d::From (AddScaled (m_controlPoints, basfuncu, m_order));
		}
	else
		{
		xyz = DPoint3d::From (0,0,0);
		dXdu = DVec3d::FromStartEnd (DPoint3d::From (0,0,0), DPoint3d::From (0,0,0));
		}
	}

void BezierCurveDPoint3d::ComputeBasisFunctionsCompact (double u, double *values) const
        { 
		if (m_order>0)
			{
			double v = 1.0 - u;
			int n = m_order - 1;
			double valPower [2][5];

			valPower[0][0] =
				valPower[1][0] = 1;
	
			for (int p=0; p<n; p++)
				{
				valPower[0][p+1]= valPower[0][p]*u;
				valPower[1][p+1]= valPower[1][p]*v;
				}

			for (int i=0; i<m_order; i++)
				{
				values[i] = s_trinomial[n][0][i]*valPower[0][i]*valPower[1][n-i];
				}

				}
			else
				{
				}
        }

void BezierCurveDPoint3d::ComputeBasisFunctionsCompact (double u, double *values, double *ddu) const
        {
		if (m_order>0)
			{
			double v = 1.0 - u;
			int n = m_order - 1;
			double valPower [2][5];

			valPower[0][0] =
				valPower[1][0] = 1;
	
			for (int p=0; p<n; p++)
				{
				valPower[0][p+1]= valPower[0][p]*u;
				valPower[1][p+1]= valPower[1][p]*v;
				}

			for (int i=0; i<m_order; i++)
				{
				values[i] = s_trinomial[n][0][i]*valPower[0][i]*valPower[1][n-i];
				ddu[i] = n*(BinomialCoefficientSafe(n-1,i-1)*valPower[0][i-1]*valPower[1][n-i]-BinomialCoefficientSafe(n-1,i)*valPower[0][i]*valPower[1][n-1-i]);
				}

				}
			else
				{
				}
        }

void BezierCurveDPoint3d::InplaceDeCasteljau (double u)
	{
	double scales[2];
	DPoint3d points[2];
	scales[0] = u;
	scales[1] = 1.0 - u;

	for (int k=0;k<m_order-1;k++)
		{
		points[0] = m_controlPoints[k];
		points[1] = m_controlPoints[k+1];
		m_controlPoints[k] = AddScaled (points, scales, 2);
		}
	m_order--;
	}

void BezierCurveDPoint3d::NoOp (double u)
	{
	}



RefCountedBezierTriangleDPoint3dBase::RefCountedBezierTriangleDPoint3dBase ()    {}
RefCountedBezierTriangleDPoint3dBase::~RefCountedBezierTriangleDPoint3dBase ()   {}
BezierTriangleDPoint3dPtr BezierTriangleDPoint3d::Create ()  {return new RefCountedBezierTriangleDPoint3d ();}

END_BENTLEY_GEOMETRY_NAMESPACE
//BezierCurve3d::>>>>>>> other
