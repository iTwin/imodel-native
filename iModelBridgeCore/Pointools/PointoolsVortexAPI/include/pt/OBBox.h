/*--------------------------------------------------------------------------*/ 
/*  Oriented Bounding Box class												*/ 
/*	Oriented Bounding Box class template									*/ 
/*  (C) 2011 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*--------------------------------------------------------------------------*/ 
#pragma once

//! Oriented Bounding Box
#include <pt/classes.h>
#include <pt/geomtypes.h>
#include <pt/BoundingBox.h>
#include <math/matrix_math.h>
#ifdef HAVE_WILDMAGIC
#include <wildmagic/math/Wm5Quaternion.h>
#endif

namespace pt
{
	template <typename T>
	class OBBox
	{
	public:
		OBBox ()	
		{	
			m_center.zero();  
			m_extents.zero();
		}
		~OBBox (){}
		
		template <typename F>
		OBBox ( const OBBox<F> &bx )					{ *this = bx; }
		
		template <typename F>
		explicit OBBox ( const BBox<F> &bx )			{ *this = bx; }

		OBBox (const vec3<T>& cen, const vec3<T> ax[3], T ext[3])
		{
			center(cen);
			m_axis[0] = ax[0];
			m_axis[1] = ax[1];
			m_axis[2] = ax[2];

			m_extents = ext;
		}
	
		OBBox (	const vec3<T>& center, const vec3<T>& axis0,
				const vec3<T>& axis1, const vec3<T>& axis2, 
					T extent0, T extent1, T extent2);

		void 				computeVertices (vec3<T> vertex[8]) const;

		void				getAABB( BoundingBoxD &box ) const;
	
		void				merge( const OBBox &box );
		
		bool 				intersects( const OBBox &box ) const;
		
		template <typename F>	
		bool 				intersects( const BBox<F> &box ) const
		{
			OBBox obox( box );
			return intersects( obox );
		}

		template <class PointType>
		bool 				contains( const PointType &pnt ) const;

		bool 				contains( const OBBox &box ) const;

		template <typename F>
		bool 				contains( const BBox<F> &bx ) const
		{
			vec3<F> corner;
			
			for (int i=0; i<8; i++) 
			{
				bx.getExtrema( i, corner);
				if (!contains( corner )) return false;
			}
			return true;
		}
		
		template <typename F>	
		bool 				containedByBBox( const BBox<F> &box ) const
		{
			return containedBy( BBox<T>(box) );
		}

		bool 				containedBy( const BBox<T> &box ) const;

		inline const vec3<T> &center() const 				{ return m_center; }
		inline const vec3<T> &axis(int a) const 			{ return m_axis[a]; }
		inline const vec3<T> &extents() const 				{ return m_extents; }
		
		void				center(const vec3<T> &c) 		{ m_center=c; }
		void				axis(int a, const vec3<T> &v) 	{ m_axis[a]=v; }		
		void				extents(const vec3<T> &e)  		{ m_extents=e; }
		void				extent(int e, T val)	 		{ m_extents[e] = val; }
		
		void				translate(const vec3<T> &t)		{ m_center += t; }
		
		void				toMatrix(mmatrix4d &mat) const
		{
			mat = mmatrix4d::identity();
			for (int i=0;i<3;i++)
			{
				mat(0,i) = m_axis[i].x;
				mat(1,i) = m_axis[i].y;
				mat(2,i) = m_axis[i].z;
			}

			double tr[] = {m_center.x, m_center.y, m_center.z, 0};
			mat >>= mmatrix4d::translation(tr);
		}

		void				fromMatrix(const mmatrix4d &mat)
		{
			for (int i=0;i<3;i++)
			{
				m_axis[i].x = mat(0,i);
				m_axis[i].y = mat(1,i);
				m_axis[i].z = mat(2,i);
				m_axis[i].normalize();
			}			
			
			m_center.set(static_cast<float>(mat(3,0)), static_cast<float>(mat(3,1)), static_cast<float>(mat(3,2)));
		}

		void				transform(const mmatrix4d &mat)
		{
			mmatrix4d m;
			toMatrix(m);
			m >>= mat;
			fromMatrix(m);
		}

		template <typename F>
		const OBBox 		&operator = ( const OBBox<F> &bx )
		{
			m_center.set( bx.center() );
			m_axis[0].set( bx.axis(0) );
			m_axis[1].set( bx.axis(1) );
			m_axis[2].set( bx.axis(2) );
			m_extents.set( bx.extents() );
			
			return (*this);
		}

		template <typename F>
		const OBBox 		&operator = ( const BBox<F> &bx )
		{
			m_center.set( bx.center() );

			m_axis[0].set(1,0,0);
			m_axis[1].set(0,1,0);
			m_axis[2].set(0,0,1);

			m_extents.set( static_cast<float>(bx.dx()*0.5), static_cast<float>(bx.dy()*0.5), static_cast<float>(bx.dz()*0.5));
			
			return (*this);
		}
	private:
		vec3<T> 			m_center;
		vec3<T> 			m_axis[3];
		vec3<T>				m_extents;
	};
	
//	typedef	OBBox<float>	OBBoxf;
	typedef	OBBox<double>	OBBoxd;

	// axis aligned box
//	OBBoxd	createFittingAABBf( const std::vector< vector3 > &pts );
//	OBBoxd	createFittingAABBd( const std::vector< vector3d > &pts );
//	OBBoxd	createFittingAABBf( const vector3 *pts, int numPoints );
	OBBoxd	createFittingAABBd(const vector3d *pts, int numPoints);

	// aligned box, uses axis in box not cardinal axis
//	bool		createFittingABBf( OBBoxd &box, const vector3 *pts, int numPoints );
//	bool		createFittingABBd( OBBoxd &box, const vector3d *pts, int numPoints );

	// oriented box, uses axis in box
//	OBBoxf	createFittingOBBf( const std::vector< vector3 > &pts );
	OBBoxd	createFittingOBBd( const std::vector< vector3d > &pts );
//	OBBoxf	createFittingOBBf( const vector3 *pts, int numPoints );
	OBBoxd	createFittingOBBd( const vector3d *pts, int numPoints );

//----------------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------------
template <typename T>
void OBBox<T>::computeVertices (vec3<T> vertex[8]) const
{
	vec3<T> ext[] = {	m_axis[0] * m_extents[0],
						m_axis[1] * m_extents[1],
						m_axis[2] * m_extents[2] };

	for (unsigned int i=0; i<8; i++)
	{
		vertex[i] = m_center;
		vertex[i] += (i & 1) ? ext[0] : - ext[0];
		vertex[i] += (i & 2) ? ext[1] : - ext[1];
		vertex[i] += (i & 4) ? ext[2] : - ext[2];
	}
}
template <typename T>
void OBBox<T>::getAABB( BoundingBoxD &box ) const
{
	vec3<T> v[8];
	computeVertices(v);

	box.clear();

	for (int i=0; i<8; i++)
	{
		box.expandBy( vector3d(v[i].x,v[i].y,v[i].z) );
	}
}
//----------------------------------------------------------------------------
template <typename T>
template <class PointType>
bool OBBox<T>::contains( const PointType &point ) const 
{
    vec3<T> diff( point.x, point.y, point.z );
	diff -= m_center;
	
    for (int i = 0; i < 3; ++i)
    {
        T coeff = diff.dot(m_axis[i]);
        if (fabs(coeff) > m_extents[i])
        {
            return false;
        }
    }
    return true;
}
//----------------------------------------------------------------------------
template <typename T>
bool OBBox<T>::containedBy( const BBox<T> &bx ) const 
{
	vec3<T> corners[8];
	computeVertices( corners );
	
	for (int i=0; i<8; i++) 
		if (!bx.inBounds( corners[i] )) return false;
	
	return true;
}
//----------------------------------------------------------------------------
template <typename T>
bool OBBox<T>::contains( const OBBox<T> &bx ) const 
{
	vec3<T> corners[8];
	computeVertices( corners );
	
	for (int i=0; i<8; i++) 
		if (!contains( corners[i] )) return false;
	
	return true;
}
//----------------------------------------------------------------------------
template <typename T>
bool OBBox<T>::intersects( const OBBox<T> &box ) const
{
    // Cutoff for cosine of angles between box axes.  This is used to catch
    // the cases when at least one pair of axes are parallel.  If this
    // happens, there is no need to test for separation along the
    // Cross(A[i],B[j]) directions.
	const T cutoff = (T)1 - 1e-06f;
    bool existsParallelPair = false;
    int i;

    // Convenience variables.
    const vec3<T>* A = m_axis;
    const vec3<T>* B = &box.axis(0);
    const T* EA = &m_extents.x;
    const T* EB = &box.extents().x;

    // Compute difference of box centers, D = C1-C0.
    vec3<T> D = box.center() - m_center;

    T C[3][3];     // matrix C = A^T B, c_{ij} = Dot(A_i,B_j)
    T AbsC[3][3];  // |c_{ij}|
    T AD[3];       // Dot(A_i,D)
    T r0, r1, r;   // interval radii and distance between centers
    T r01;         // = R0 + R1

    // axis C0+t*A0
    for (i = 0; i < 3; ++i)
    {
        C[0][i] = A[0].dot(B[i]);
        AbsC[0][i] = fabs(C[0][i]);
        if (AbsC[0][i] > cutoff)
        {
            existsParallelPair = true;
        }
    }
    AD[0] = A[0].dot(D);
    r = fabs(AD[0]);
    r1 = EB[0]*AbsC[0][0] + EB[1]*AbsC[0][1] + EB[2]*AbsC[0][2];
    r01 = EA[0] + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A1
    for (i = 0; i < 3; ++i)
    {
        C[1][i] = A[1].dot(B[i]);
        AbsC[1][i] = fabs(C[1][i]);
        if (AbsC[1][i] > cutoff)
        {
            existsParallelPair = true;
        }
    }
    AD[1] = A[1].dot(D);
    r = fabs(AD[1]);
    r1 = EB[0]*AbsC[1][0] + EB[1]*AbsC[1][1] + EB[2]*AbsC[1][2];
    r01 = EA[1] + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A2
    for (i = 0; i < 3; ++i)
    {
        C[2][i] = A[2].dot(B[i]);
        AbsC[2][i] = fabs(C[2][i]);
        if (AbsC[2][i] > cutoff)
        {
            existsParallelPair = true;
        }
    }
    AD[2] = A[2].dot(D);
    r = fabs(AD[2]);
    r1 = EB[0]*AbsC[2][0] + EB[1]*AbsC[2][1] + EB[2]*AbsC[2][2];
    r01 = EA[2] + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*B0
    r = fabs(B[0].dot(D));
    r0 = EA[0]*AbsC[0][0] + EA[1]*AbsC[1][0] + EA[2]*AbsC[2][0];
    r01 = r0 + EB[0];
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*B1
    r = fabs(B[1].dot(D));
    r0 = EA[0]*AbsC[0][1] + EA[1]*AbsC[1][1] + EA[2]*AbsC[2][1];
    r01 = r0 + EB[1];
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*B2
    r = fabs(B[2].dot(D));
    r0 = EA[0]*AbsC[0][2] + EA[1]*AbsC[1][2] + EA[2]*AbsC[2][2];
    r01 = r0 + EB[2];
    if (r > r01)
    {
        return false;
    }

    // At least one pair of box axes was parallel, so the separation is
    // effectively in 2D where checking the "edge" normals is sufficient for
    // the separation of the boxes.
    if (existsParallelPair)
    {
        return true;
    }

    // axis C0+t*A0xB0
    r = fabs(AD[2]*C[1][0] - AD[1]*C[2][0]);
    r0 = EA[1]*AbsC[2][0] + EA[2]*AbsC[1][0];
    r1 = EB[1]*AbsC[0][2] + EB[2]*AbsC[0][1];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A0xB1
    r = fabs(AD[2]*C[1][1] - AD[1]*C[2][1]);
    r0 = EA[1]*AbsC[2][1] + EA[2]*AbsC[1][1];
    r1 = EB[0]*AbsC[0][2] + EB[2]*AbsC[0][0];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A0xB2
    r = fabs(AD[2]*C[1][2] - AD[1]*C[2][2]);
    r0 = EA[1]*AbsC[2][2] + EA[2]*AbsC[1][2];
    r1 = EB[0]*AbsC[0][1] + EB[1]*AbsC[0][0];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A1xB0
    r = fabs(AD[0]*C[2][0] - AD[2]*C[0][0]);
    r0 = EA[0]*AbsC[2][0] + EA[2]*AbsC[0][0];
    r1 = EB[1]*AbsC[1][2] + EB[2]*AbsC[1][1];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A1xB1
    r = fabs(AD[0]*C[2][1] - AD[2]*C[0][1]);
    r0 = EA[0]*AbsC[2][1] + EA[2]*AbsC[0][1];
    r1 = EB[0]*AbsC[1][2] + EB[2]*AbsC[1][0];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A1xB2
    r = fabs(AD[0]*C[2][2] - AD[2]*C[0][2]);
    r0 = EA[0]*AbsC[2][2] + EA[2]*AbsC[0][2];
    r1 = EB[0]*AbsC[1][1] + EB[1]*AbsC[1][0];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A2xB0
    r = fabs(AD[1]*C[0][0] - AD[0]*C[1][0]);
    r0 = EA[0]*AbsC[1][0] + EA[1]*AbsC[0][0];
    r1 = EB[1]*AbsC[2][2] + EB[2]*AbsC[2][1];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A2xB1
    r = fabs(AD[1]*C[0][1] - AD[0]*C[1][1]);
    r0 = EA[0]*AbsC[1][1] + EA[1]*AbsC[0][1];
    r1 = EB[0]*AbsC[2][2] + EB[2]*AbsC[2][0];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A2xB2
    r = fabs(AD[1]*C[0][2] - AD[0]*C[1][2]);
    r0 = EA[0]*AbsC[1][2] + EA[1]*AbsC[0][2];
    r1 = EB[0]*AbsC[2][1] + EB[1]*AbsC[2][0];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    return true;
}
template <typename T>
void OBBox<T>::merge( const OBBox<T>& box1 )
{
    // The first guess at the box center.  This value will be updated later
    // after the input box vertices are projected onto axes determined by an
    // average of box axes.
    center( (m_center + box1.center())*(T)0.5 );

    // A box's axes, when viewed as the columns of a matrix, form a rotation
    // matrix.  The input box axes are converted to quaternions.  The average
    // quaternion is computed, then normalized to unit length.  The result is
    // the slerp of the two input quaternions with t-value of 1/2.  The result
    // is converted back to a rotation matrix and its columns are selected as
    // the merged box axes.
#ifdef HAVE_WILDMAGIC
	Wm5::Quaternion<T> q0, q1;
    q0.FromRotationMatrix(m_axis);
    q1.FromRotationMatrix(box1.m_axis);

    if (q0.Dot(q1) < (T)0)
    {
        q1 = -q1;
    }

    Wm5::Quaternion<T> q = q0 + q1;
	T invLength = Wm5::Math<T>::InvSqrt(q.Dot(q));
    q = invLength*q;
    q.ToRotationMatrix(&m_axis[0].x);
#else
    // &&RB TODO: replace wilmagic function with geomlibs function
    //DPoint4d q0, q1;
    // &&RB TODO: the following geomlibs function call must be tested
#endif

    // Project the input box vertices onto the merged-box axes.  Each axis
    // D[i] containing the current center C has a minimum projected value
    // min[i] and a maximum projected value max[i].  The corresponding end
    // points on the axes are C+min[i]*D[i] and C+max[i]*D[i].  The point C
    // is not necessarily the midpoint for any of the intervals.  The actual
    // box center will be adjusted from C to a point C' that is the midpoint
    // of each interval,
    //   C' = C + sum_{i=0}^2 0.5*(min[i]+max[i])*D[i]
    // The box extents are
    //   e[i] = 0.5*(max[i]-min[i])

    int i, j;
    T dot;
    vec3<T> vertex[8], diff;
    vec3<T> pmin;
    vec3<T> pmax;

	pmin.zero();
	pmax.zero();
	
    computeVertices(vertex);
    for (i = 0; i < 8; ++i)
    {
        diff = vertex[i] - box1.center();
        for (j = 0; j < 3; ++j)
        {
            dot = diff.dot(m_axis[j]);
            if (dot > pmax[j])
            {
                pmax[j] = dot;
            }
            else if (dot < pmin[j])
            {
                pmin[j] = dot;
            }
        }
    }

    box1.computeVertices(vertex);
    for (i = 0; i < 8; ++i)
    {
        diff = vertex[i] - box1.center();
        for (j = 0; j < 3; ++j)
        {
            dot = diff.dot(box1.m_axis[j]);
            if (dot > pmax[j])
            {
                pmax[j] = dot;
            }
            else if (dot < pmin[j])
            {
                pmin[j] = dot;
            }
        }
    }

    // [min,max] is the axis-aligned box in the coordinate system of the
    // merged box axes.  Update the current box center to be the center of
    // the new box.  Compute the extents based on the new center.
    for (j = 0; j < 3; ++j)
    {
		vec3<T> t( (pmax[j] + pmin[j]) * m_axis[j] );
        translate(  t* (T)0.5 );

        extent(j, (pmax[j] - pmin[j])*(T)0.5);
    }
}
};	// namespace