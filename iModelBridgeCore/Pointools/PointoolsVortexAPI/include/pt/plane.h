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

#include <pt/geomtypes.h>
#include <math/matrix_math.h>
#include <pt/boundingbox.h>

namespace pt
{
template<class T>
class Plane
{
public:
	typedef vec3<T>			VectorType;
	typedef matrix<3,3,T>	MatrixType;
	Plane()
	{
		m_normal.x = 0;
		m_normal.y = 0;
		m_normal.z = 1.0;
		m_constant = 0;
		m_transform = MatrixType::identity();
		m_transform_dirty = true;
	}
	Plane(const Plane<T> &p)
	{
		normal(p.normal());
		base(p.base());
		m_transform_dirty = true;
	}
	Plane(const VectorType &base, const VectorType &normal) : m_base(base), m_normal(normal)
	{
		m_normal = normal;
		m_constant = normal.dot(base);
		m_base = base;
		m_transform = MatrixType::identity();
		m_transform_dirty = true;
	}
	Plane(const VectorType &normal, T k) : m_normal(normal)
	{
		constant(k);
		m_transform_dirty = true;
	}
	Plane(const VectorType &a, const VectorType &b, const VectorType &c)
	{
		m_transform = MatrixType::identity();
		from3points(a,b,c);
	}
	~Plane() {};

	/*access*/ 	
	/* plane normal												*/ 
	inline void normal(const VectorType &n)		{ m_normal = n; m_transform_dirty = true; }
	inline const VectorType	&normal() const		{ return m_normal; }

	/* plane's constant											*/ 
	inline void constant(T k)		{ m_constant = k; m_base = m_normal * k; m_transform_dirty = true; }
	inline float constant() const	{ return m_constant; }

	/* plane's base position									*/ 
	/* use instead of constant for simplified positioning		*/ 
	void base(const VectorType &v)	{
		m_base = v; 
		m_constant = m_normal.dot(m_base); 
		m_transform_dirty = true; 
	}
	const VectorType &base() const	{ return m_base; }

	/* move the plane's position								*/ 
	void move(const VectorType &t)		{ base(m_base + t); }

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
	void from3points(const VectorType &a, const VectorType &b, const VectorType &c)
	{
		VectorType edge1 = b - a;
		VectorType edge2 = c - a;
		m_normal = edge1.cross(edge2);
		m_normal.normalize();
		m_constant = m_normal.dot(a);
		m_base = a;
		m_transform_dirty = true;
	}

	/*intersection of ray with plane - point of intersection returned	*/ 
	bool intersectRay(const VectorType &origin, const VectorType &dir, T *res) const
	{
		T t = -1;
		T d = m_normal.dot(dir);

		if (d != 0)
		{
			VectorType plane_origin = m_normal * m_constant;
			t = m_normal.dot(plane_origin - origin) / d;
			(origin + dir * t).get(res);
			return true;
		}
		return false;
	}
	/*intersection of line with plane - point of intersection returned	*/ 
	bool intersectLine(const VectorType &a, const VectorType &b, VectorType &i) const
	{
		/*check endpoints are either side of plane, then do ray intersection*/ 
		return (whichSide(a) != whichSide(b) ? intersectRay(a, b - a, i) : false);
	}
	/*intersection test of box and plane								*/ 
	bool intersects(const BoundingBox &bb) const
	{
		VectorType v;

		bb.getExtrema(0, v);
		int side = whichSide(v);

		for (int i=1; i< 8; i++)
		{
			bb.getExtrema(i, v);
			if (whichSide(v) != side) return true;
		}
		return false;
	}
	/* Perpendicular projection to plane	 							*/ 
	bool projectToPlane( T *v ) const
	{
		VectorType o(v);
		return ( intersectRay(o, m_normal, v) );
	}
	/* Perpendicular projection to plane returned in 2d plane coords	*/ 
	bool projectToPlane(const VectorType &v, vec2<T> &v2) const
	{
		VectorType o(v), v1;
		if (intersectRay(o, m_normal, v1))
		{
			to2D(v1, v2);
			return true;
		}
		return false;
	}
	/* transform a point into the planes coordinate system				*/ 
	void transformIntoPlaneCS(VectorType &v) const
	{
		if (m_transform_dirty)
			const_cast<Plane*>(this)->updateTransformData();

		m_transform.mat3_multiply_vec3(v);
		v += m_base;	
	}
	/* define from Transformation matrix								*/ 
	void fromTransformMatrix(const matrix<4,4,T> &mat)
	{
		MatrixType m;
		int i, j;
		for (i=0; i<3; i++)
			for (j=0; j<3; j++)
				m(i,j) = mat(i,j);

		m.orthoNormalize();
		normal(VectorType(m(2,0), m(2,1), m(2,2)));
		base(VectorType(mat(3,0), mat(3,1), mat(3,2)));
	}
	/*																	*/ 
	inline const MatrixType &rotationMatrix() const 
	{ 
		if (m_transform_dirty)
			const_cast<Plane*>(this)->updateTransformData(); 

		return m_transform; 
	};

#if defined(NEEDS_WORK_VORTEX_DGNDB)
	void getTransformMatrix(matrix<4,4,T> &m) const
	{
		m = matrix<4,4,T>::identity();

		for (uint32_t i=0; i<3; i++)
			for (uint32_t j=0; j<3; j++)
				mat(i,j) = m(i,j);

		mat(3,0) = m_base.x;
		mat(3,1) = m_base.y;
		mat(3,2) = m_base.z;
	}
#endif

	/* converting 2D plane coords into 3D coords						*/ 
	void to3D(const vec2<T> &p, VectorType &t) const
	{	
		/*use u,v basis vectors for 2d frame*/ 
		VectorType u, v;

		extractUV(u,v);

		u.x *= p.x;
		u.y *= p.x;
		u.z *= p.x;

		v.x *= p.y;
		v.y *= p.y;
		v.z *= p.y;

		/*reuse u for effieciency*/ 
		u += v;
		u += m_base;
		t = u;
	}
	/* converting 3D coords into 2D plane coords						*/ 
	void to2D(const VectorType &p, vec2<T> &t) const
	{
		/*use u,v basis vectors for 2d frame*/ 
		VectorType u, v;
		VectorType q(p);

		extractUV(u,v);

		q -= m_base;

		t.x = u.dot(q);
		t.y = v.dot(q);
	}
	/* converting 3D coords into 2D coords with depth value in z		*/ 
	void to2D(const VectorType &p, VectorType &t) const
	{
		/*use u,v basis vectors for 2d frame*/ 
		VectorType u, v;
		VectorType q(p);

		extractUV(u,v);

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
	int whichSide(const VectorType &v) const
	{
		T d = distToPlane(v);
		if ( d < 0 ) return -1;
		if ( d > 0 ) return 1;
		return 0;		
	}
	int whichSideWithTolerance(const VectorType &v, T tol) const
	{
		T d = distToPlane(v);
		if ( d < -tol ) return -1;
		if ( d > tol ) return 1;
		return 0;		
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
	inline void extractUV(VectorType &u, VectorType &v) const
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
		VectorType u, v;
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
	VectorType		m_normal;
	VectorType		m_base;
	MatrixType		m_transform;
};

typedef Plane<float>	Planef;
typedef Plane<double>	Planed;
}
#endif