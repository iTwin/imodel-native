//----------------------------------------------------------------------------
//
// Plane.h
//
// Copyright (c) 2014 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------

#ifndef POINTOOLS_EXAMPLES_PLANE_H
#define POINTOOLS_EXAMPLES_PLANE_H

#include "matrix_math.h"


namespace vortex
{

template<class T>
class Plane
{
public:
	Plane()
	{
		m_normal.x = 0;
		m_normal.y = 0;
		m_normal.z = 1.0;
		m_constant = 0;
		m_transform = mmatrix4d::identity();
		m_transform_dirty = true;
	}
	Plane(const Plane<T> &p)
	{
		normal(p.normal());
		base(p.base());
		m_transform_dirty = true;
	}
	Plane(const Vector3<T> &base, const Vector3<T> &normal) : m_base(base), m_normal(normal)
	{
		m_normal = normal;
		m_constant = normal.dot(base);
		m_base = base;
		m_transform = matrix::identity();
		m_transform_dirty = true;
	}
	Plane(const Vector3<T> &normal, T k) : m_normal(normal)
	{
		constant(k);
		m_transform_dirty = true;
	}
	Plane(const Vector3<T> &a, const Vector3<T> &b, const Vector3<T> &c)
	{
		m_transform = MatrixType::identity();
		from3points(a,b,c);
	}
	~Plane() {};

	/*access*/ 	
	/* plane normal												*/ 
	inline void normal(const Vector3<T> &n)		{ m_normal = n; m_transform_dirty = true; }
	inline const Vector3<T>	&normal() const		{ return m_normal; }

	/* plane's constant											*/ 
	inline void constant(T k)		{ m_constant = k; m_base = m_normal * k; m_transform_dirty = true; }
	inline float constant() const	{ return static_cast<float>(m_constant); }

	/* plane's base position									*/ 
	/* use instead of constant for simplified positioning		*/ 
	void base(const Vector3<T> &v)	{
		m_base = v; 
		m_constant = m_normal.dot(m_base); 
		m_transform_dirty = true; 
	}
	const Vector3<T> &base() const	{ return m_base; }

	/* move the plane's position								*/ 
	void move(const Vector3<T> &t)		{ base(m_base + t); }

	/* U, V vectors of plane									*/ 
	void getUV(Vector3<T> &u, Vector3<T> &v) const
	{
		const Vector3<T> &n = m_normal;

		if (fabs(n.x) >= fabs(n.y))
		{
			/* n.x or n.z is the largest magnitude component*/ 
			T invLength = 1 / sqrt(n.x * n.x + n.z * n.z);
			u.x = n.z * invLength;
			u.y = 0;
			u.z = -n.x * invLength;
		}
		else
		{
			/*n.y or n.z is the largest magnitude component*/ 
			T invLength = 1 / sqrt(n.y * n.y + n.z * n.z);
			u.x = 0;
			u.y = n.z * invLength;
			u.z = -n.y * invLength;
		}
		v = n.cross(u);		
	}
	/* normalize normal to unit length - is invariant			*/ 
	inline void normalize() 
	{    
		T l = (T)1.0/m_normal.length();
		m_normal.x *= l;
		m_normal.y *= l;
		m_normal.z *= l;
		m_constant *= l; 
	}

	/*conversion												*/ 
	void from3points(const Vector3<T> &a, const Vector3<T> &b, const Vector3<T> &c)
	{
		Vector3<T> edge1 = b - a;
		Vector3<T> edge2 = c - a;
		m_normal = edge1.cross(edge2);
		m_normal.unitize();
		m_constant = m_normal.dot(a);
		m_base = (b+c)/2;

		updateTransformData();
	}

	void update(const Vector3<T>& v, T k)
	{
		m_normal = v;
		m_normal.unitize();
		m_constant = k;
		m_base = m_normal * k;

		updateTransformData();
	}

	/* converting 2D plane coords into 3D coords						*/ 
	void to3D(T px, T py, Vector3<T> &t) const
	{	
		/*use u,v basis vectors for 2d frame*/ 
		Vector3<T> u, v;

		extractUV(u,v);

		u.x *= px;
		u.y *= px;
		u.z *= px;

		v.x *= py;
		v.y *= py;
		v.z *= py;

		/*reuse u for effieciency*/ 
		u += v;
		u += m_base;
		t = u;
	}
	const Plane<T> &operator = (const Plane<T> &p)
	{
		normal(p.normal());
		base(p.base());
		return (*this);
	}
	int whichSide(const Vector3<T> &v) const
	{
		T d = distToPlane(v);
		if ( d < 0 ) return -1;
		if ( d > 0 ) return 1;
		return 0;		
	}
	inline T distToPlane(const Vector3<T> &v) const
	{ 
		return m_normal.dot(v) - m_constant; 
	}

private:
	inline void extractUV(Vector3<T> &u, Vector3<T> &v) const
	{
		if (m_transform_dirty)
			const_cast<Plane*>(this)->updateTransformData();

		u.x = m_transform(0,0);
		u.y = m_transform(1,0);
		u.z = m_transform(2,0);

		v.x = m_transform(0,1);
		v.y = m_transform(1,1);
		v.z = m_transform(2,1);
	}

	void	updateTransformData()
	{
		/*make sure plane is normalized before doing this*/ 
		Vector3<T> u, v;
		getUV(v,u);

		/*u,v and n form cols of rotation matrix*/ 
		m_transform(0,0) = u.x;
		m_transform(1,0) = u.y;
		m_transform(2,0) = u.z;

		m_transform(0,1) = v.x;
		m_transform(1,1) = v.y;
		m_transform(2,1) = v.z;

		m_transform(0,2) = m_normal.x;
		m_transform(1,2) = m_normal.y;
		m_transform(2,2) = m_normal.z;

		m_transform_dirty = false;
	}

	bool			m_transform_dirty;

	T				m_constant;
	Vector3<T>		m_normal;
	Vector3<T>		m_base;
	mmatrix4d		m_transform;
};

typedef Plane<float>	Planef;
typedef Plane<double>	Planed;

}

#endif // POINTOOLS_EXAMPLES_PLANE_H
