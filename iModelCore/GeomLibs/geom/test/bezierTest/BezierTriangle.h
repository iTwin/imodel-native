/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*__PUBLISH_SECTION_END__*/

struct BezierTriangle1;
typedef BezierTriangle1 &BezierTriangle1R;
typedef BezierTriangle1 const &BezierTriangle1CR;
struct BezierTriangle2;
typedef BezierTriangle2 &BezierTriangle2R;
typedef BezierTriangle2 const &BezierTriangle2CR;
struct BezierTriangle3;
typedef BezierTriangle3 &BezierTriangle3R;
typedef BezierTriangle3 const &BezierTriangle3CR;


struct SurfaceCoordinateDetail;
typedef SurfaceCoordinateDetail &SurfaceCoordinateDetailR;
typedef SurfaceCoordinateDetail const &SurfaceCoordinateDetailCR;


struct SurfaceCoordinateDetail
{
DPoint2d uv;
DPoint3d xyz;
DVec3d   dXdu;
DVec3d   dXdv;
DVec3d   dXduu;
DVec3d   dXdvv;
DVec3d   dXduv;
};

//! Static class with degree-specific methods for basis function evaluation (independent of how vertices are stored)
//!
struct BezierBasisFunctions
{
private:
BezierBasisFunctions ();
public:
static void EvaluateBasis1 (double u, double v, double *basisValues);
static void EvaluateBasis2 (double u, double v, double *basisValues);
static void EvaluateBasis3 (double u, double v, double *basisValues);
static void EvaluateBasis4 (double u, double v, double *basisValues);

static void EvaluateBasis1 (double u, double v, double *basisValues, double *dFdu, double *dFdv);
static void EvaluateBasis2 (double u, double v, double *basisValues, double *dFdu, double *dFdv);
static void EvaluateBasis3 (double u, double v, double *basisValues, double *dFdu, double *dFdv);
static void EvaluateBasis4 (double u, double v, double *basisValues, double *dFdu, double *dFdv);

static void EvaluateBasis1 (double u, double v, double *basisValues, double *dFdu, double *dFdv, double *dFduu, double *dFdvv, double *dFduv);
static void EvaluateBasis2 (double u, double v, double *basisValues, double *dFdu, double *dFdv, double *dFduu, double *dFdvv, double *dFduv);
static void EvaluateBasis3 (double u, double v, double *basisValues, double *dFdu, double *dFdv, double *dFduu, double *dFdvv, double *dFduv);
static void EvaluateBasis4 (double u, double v, double *basisValues, double *dFdu, double *dFdv, double *dFduu, double *dFdvv, double *dFduv);

};
//! Bezier triangle up to degree MaxDegree.
//! The structure declares enough space of the MaxDegree triangle.
//! (If you are making lots of low degree triangles, this is wasting space, but is it critical to anything?)
struct BezierTriangle
{
unsigned int degree;
static const int MaxDegree = 4;
static const int MaxcontrolPoints = 15;   
DPoint3d m_controlPoints[MaxcontrolPoints];

void SetLinear (DPoint3d *controlPoints);
void SetQuadratic (DPoint3d *controlPoints);
void SetCubic (DPoint3d *controlPoints);
void SetQuartic (DPoint3d *controlPoints);
//! install controlPoints.
//! @return false if the array size is not one of the acceptable sizes (3,6,10,15)
bool Set (bvector<DPoint3d> const &controlPoints);
//! @description return the degree of the polynomials
unsigned int GetDegree () const;

void EvaluateBasis (double u, double v, double *basisValues) const;
void EvaluateBasis (double u, double v, double *basisValues, double *dFdu, double *dFdv) const;
void EvaluateBasis (double u, double v, double *basisValues, double *dFdu, double *dFdv, double *dFduu, double *dFdvv, double *dFduv) const;
DPoint3d Evaluate (double u, double v) const;
DPoint3d Evaluate (double u, double v);
void Evaluate (DPoint2dCR uv, SurfaceCoordinateDetailR data, int numDerivative = 2) const;
};

// abstract class defining virtual methods for evaluation of a surface that is a function of barycentric coordinates.
struct BarycentricTriangle
{
public:
//! @param [in] uvw barycentric coordinates for evaluation
//! @return evaluated point
//! @remark this method may assumes u+v+w=1.
virtual DPoint3d Evaluate (DPoint3dCR uvw) = 0;
//! Evaluate at (u,v,1-u-v)
//! @param [in] u barycentric u coordinate
//! @param [in] v barycentric v coordinate
virtual DPoint3d Evaluate (double u, double v) = 0;
//! Evaluate point and derivatives at (u,v,1-u-v)
//! @param [in] u barycentric u coordinate
//! @param [in] v barycentric v coordinate
//! @param [out] xyz xyz coordinates
//! @param [out] dXdu derivative wrt u (v fixed, w varies with dw = -du?)
//! @param [out] dXdv derivative wrt v (v fixed, w varies with dw = -dv?)
virtual void Evaluate (double u, double v, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) = 0;

};




//! Degree 1 bezier triangle.
//! 3 points.
//! u varies fastest from xyz00 to xyz10 (This is v = 0)
//! v varies fastest from xyz00 to xyz01 (This is u = 0)
struct BezierTriangle1 : public BarycentricTriangle
{
private:
public:
DPoint3d m_poles[3];
virtual DPoint3d Evaluate (DPoint3dCR uvw) override;
virtual DPoint3d Evaluate (double u, double v) override;
virtual void Evaluate (double u, double v, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) override;
public:
//! Constructor -- points given as separate arguments.
BezierTriangle1 (DPoint3dCR xyz00, DPoint3dCR xyz10, DPoint3dCR xyz01);
//! Constructor -- points given as bvector<>. Order matches the 3-arg constructor.
BezierTriangle1 (bvector<DPoint3d> &points);
//! Constructor -- contiguous points.
BezierTriangle1 (DPoint3d *points);
//! Return a (new) barycentric triangle that is the transformation of given triangle.

static BezierTriangle1 FromMultiply (TransformCR transform, BezierTriangle1CR triangle);


//! Multiply the instance (in place)
void Multiply (TransformCR transform);

void ComputeBasisFunctions (double u, double v, double* values);
void ComputeBasisFunctions (double u, double v, double* values, double* ddu, double* ddv);
};

//! Degree 2 bezier triangle.
//! 6 points.
//!     xyz02
//!         |\
//!         | \
//!         |  \
//!         |   \
//!     xyz01    xyz11
//!         |     \
//!         |      \
//!         |       \
//!         |        \
//!     xyz00--xyz10--xyz20
//!
struct BezierTriangle2 : BarycentricTriangle
{
private:
public:
DPoint3d m_poles[6];
virtual DPoint3d Evaluate (DPoint3dCR uvw) override;
virtual DPoint3d Evaluate (double u, double v) override;
virtual void Evaluate (double u, double v, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) override;
static BezierTriangle2 FromMultiply (TransformCR transform, BezierTriangle2CR triangle);
void Multiply (TransformCR transform);
void ComputeBasisFunctions (double u, double v, double* values);
void ComputeBasisFunctions (double u, double v, double* values, double* ddu, double* ddv);
public:
BezierTriangle2 (
DPoint3dCR xyz00, 
DPoint3dCR xyz10, 
DPoint3dCR xyz20,
DPoint3dCR xyz01,
DPoint3dCR xyz11,
DPoint3dCR xyz02
);
//! Constructor -- points given as bvector<>. Order matches the 6-arg constructor.
BezierTriangle2 (bvector<DPoint3d> &points);
BezierTriangle2 (DPoint3d *points);
};

//! Degree 3 bezier triangle.
//! 10 points.
//!                  xyz03						
//!    				  /\				
//!  			     /  \						
//!   			    /	 \ 						
//!              xyz02  xyz12
//!     	      /		   \				
//! 			 /	        \
//!           xyz01 xyz11 xyz21
//!            /              \
//!    		  /                \
//!	       xyz00-xyz10--xyz20-xyz30
//!
struct BezierTriangle3 : BarycentricTriangle
{
private:
public:
DPoint3d m_poles[10];
virtual DPoint3d Evaluate (DPoint3dCR uvw) override;
virtual DPoint3d Evaluate (double u, double v) override;
virtual void Evaluate (double u, double v, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) override;
static BezierTriangle3 FromMultiply (TransformCR transform, BezierTriangle3CR triangle);
void Multiply (TransformCR transform);
void ComputeBasisFunctions (double u, double v, double* values);
void ComputeBasisFunctions (double u, double v, double* values, double* ddu, double* ddv);
public:
BezierTriangle3 (
DPoint3dCR xyz00, 
DPoint3dCR xyz10, 
DPoint3dCR xyz20,
DPoint3dCR xyz30,
DPoint3dCR xyz01,
DPoint3dCR xyz11,
DPoint3dCR xyz21,
DPoint3dCR xyz02,
DPoint3dCR xyz12,
DPoint3dCR xyz03
);
//! Constructor -- points given as bvector<>. Order matches the 6-arg constructor.
BezierTriangle3 (bvector<DPoint3d> &points);
BezierTriangle3 (DPoint3d *points);
};

/*__PUBLISH_SECTION_START__*/
END_BENTLEY_GEOMETRY_NAMESPACE