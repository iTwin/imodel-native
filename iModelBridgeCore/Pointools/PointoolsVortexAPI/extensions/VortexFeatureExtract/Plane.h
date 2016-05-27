/*--------------------------------------------------------------------------*/ 
/*  Plane.h																	*/ 
/*	Plane class definition													*/ 
/*  (C) 2004 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 1 Feb 2005 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
/*
 * Plane class designed for easy of definition, placement, projection and 
 * coordinate space transformation suited to editing environments
 * Not designed to be compact form
 *
*/ 
#ifndef POINTOOLS_PLANE_CLASS
#define POINTOOLS_PLANE_CLASS 1

#include "GeomTypes.h"

namespace vortex
{

template<class T>
class Plane
{
public:
	typedef Vector3<T>			VectorType;
	typedef Point3<T>			PointType;

	Plane()
	{
		m_normal.x = 0;
		m_normal.y = 0;
		m_normal.z = 1.0;
		m_constant = 0;
	}
	Plane(const Plane<T> &p)
	{
		normal(p.normal());
		base(p.base());
	}
	Plane(const PointType &base, const VectorType &normal) : m_base(base), m_normal(normal)
	{
		m_normal = normal;
		m_constant = normal.dot(VectorType(base.x,base.y,base.z));
		m_base = base;
	}
	Plane(const VectorType &normal, T k) : m_normal(normal)
	{
		constant(k);
	}
	Plane(const VectorType &a, const VectorType &b, const VectorType &c)
	{
		from3points(a,b,c);
	}
	~Plane() {};

	/*access*/ 	
	/* plane normal												*/ 
	inline void normal(const VectorType &n)		{ m_normal = n; /*m_transform_dirty = true;*/ }
	inline const VectorType	&normal() const		{ return m_normal; }

	/* plane's constant											*/ 
	inline void constant(T k)		{ m_constant = k; m_base = m_normal * k; /*m_transform_dirty = true;*/ }
	inline T constant() const		{ return m_constant; }

	/* plane's base position									*/ 
	/* use instead of constant for simplified positioning		*/ 
	void base(const VectorType &v)	
	{
		m_base = v; 
		m_constant = m_normal.dot(m_base); 
	}
	const VectorType &base() const	
	{ 
		return m_base; 
	}

	/* move the plane's position								*/ 
	void move(const VectorType &t)		
	{ 
		base(m_base + t); 
	}

	/* U, V vectors of plane									*/ 
	void getUV(VectorType &u, VectorType &v) const
	{
		const VectorType &n = m_normal;

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
	void from3points(const PointType &a, const PointType &b, const PointType &c)
	{
		VectorType edge1 = b - a;
		VectorType edge2 = c - a;
		m_normal = edge1.cross(edge2);
		m_normal.normalize();
		m_constant = m_normal.dot(a);
		m_base = a;
	}

	/*intersection of ray with plane - point of intersection returned	*/ 
	bool intersectRay(const PointType &origin, const VectorType &dir, PointType &res) const
	{
		T t = -1;
		T d = m_normal.dot(dir);

		if (d != 0)
		{
			VectorType plane_origin = m_normal * m_constant;
			t = m_normal.dot(plane_origin - origin) / d;
			res = (origin + dir * t);
			return true;
		}
		return false;
	}
	/*intersection of line with plane - point of intersection returned	*/ 
	bool intersectLine(const PointType &a, const PointType &b, PointType &i) const
	{
		/*check endpoints are either side of plane, then do ray intersection*/ 
		return (whichSide(a) != whichSide(b) ? intersectRay(a, b - a, i) : false);
	}
	/* Perpendicular projection to plane	 							*/ 
	bool projectToPlane( PointType &p ) const
	{
		PointType o(v);
		return ( intersectRay(o, m_normal, v) );
	}
	/* Perpendicular projection to plane returned in 2d plane coords	*/ 
	bool projectToPlane(const PointType &v, PointType &v2) const
	{
		VectorType o(v), v1;
		if (intersectRay(o, m_normal, v1))
		{
			to2D(v1, v2);
			return true;
		}
		return false;
	}
	/* converting 2D plane coords into 3D coords						*/ 
	void to3D(const PointType &p, PointType &t) const
	{	
		/*use u,v basis vectors for 2d frame*/ 
		VectorType u, v;

		getUV(u,v);

		u.x *= p.x;
		u.y *= p.x;
		u.z *= p.x;

		v.x *= p.y;
		v.y *= p.y;
		v.z *= p.y;

		/*reuse u for efficiency*/ 
		u += v;
		u += m_base;

		t.set(u.x,u.y,u.z);
	}

	/* converting 3D coords into 2D coords with depth value in z		*/ 
	void to2D(const PointType &p, PointType &t) const
	{
		/*use u,v basis vectors for 2d frame*/ 
		VectorType u, v;
		VectorType q(p);

		getUV(u,v);

		q -= m_base;

		t.x = u.dot(q);
		t.y = v.dot(q);
		t.z = distToPlane(p);
	}

	const Plane<T> &operator = (const Plane<T> &p)
	{
		normal(p.normal());
		base(p.base());
		return (*this);
	}

	int whichSide(const PointType &v) const
	{
		T d = distToPlane(v);
		if ( d < 0 ) return -1;
		if ( d > 0 ) return 1;
		return 0;		
	}

	inline T distToPlane(const PointType &v) const
	{ 
		return m_normal.dot(v) - m_constant; 
	}

	inline T distToPlane(const VectorType &v) const
	{ 
		return m_normal.dot(v) - m_constant; 
	}

	template <class To> void
		convert(Plane<To> &p)
	{
		p.normal(vec3<To>(normal()));
		p.base(vec3<To>(base()));
	}
private:

	T				m_constant;
	VectorType		m_normal;
	PointType		m_base;
};

typedef Plane<float>	Planef;
typedef Plane<double>	Planed;
}
#endif