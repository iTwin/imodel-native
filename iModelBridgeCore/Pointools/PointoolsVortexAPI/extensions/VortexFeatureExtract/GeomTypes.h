//----------------------------------------------------------------------------
//
// VortexFeatureExtract.h
//
// Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
#ifndef POINTOOLS_VORTEX_GEOM_TYPES_API_H
#define POINTOOLS_VORTEX_GEOM_TYPES_API_H

#define PT_EPS 0.0000001

#include <math.h>

namespace vortex
{

//
// Vector3 class
//
template <typename T>
class Vector3
{
public:
	T x;
	T y;
	T z;

	typedef	Vector3<T> Vec;

	/*constructors*/ 
	Vector3<T>() : x(0), y(0), z(0) {};

	Vector3<T>(const T*v)		
	{ 
		x = v[0]; y = v[1]; z = v[2]; 
	}

	Vector3<T>(const Vec &a, const Vec &b) 
	{ 
		*this = b; 
		*this -= a; 
	}

	Vector3<T>(const T &nx, const T &ny, const T &nz)
	{
		x = nx; 
		y = ny;	
		z = nz;
	}

	template<class P>
	Vector3<T>(const P &p) : x(p.x), y(p.y), z(p.z) {}

	/*copy constructor*/ 
	Vector3<T>(const Vec& v) 
	{ 
		*this = v; 
	}

	/*accessors*/ 
	inline operator const T*() const	
	{ 
		return ((const T*)this); 
	}
	inline operator T *()				
	{ 
		return ((T*)this); 
	}

	/* operators*/ 
	inline bool operator == (const Vec& v) const
	{
		return (fabs(v.x-x)<PT_EPS &&
				fabs(v.y-y)<PT_EPS &&
				fabs(v.z-z)<PT_EPS) ? true : false;
	}

	inline Vec operator - () const 
	{
		return Vec(-x, -y, -z); 
	}

	inline Vec operator - (const Vec& vx) const 
	{
		return Vec(x - vx.x, y - vx.y, z - vx.z); 
	};

	inline Vec operator + (const Vec& vx) const 
	{
		return Vec(x + vx.x, y + vx.y, z + vx.z); 
	};

	inline Vec operator * (const Vec& vx) const 
	{
		return Vec(x * vx.x, y * vx.y, z * vx.z); 
	};

	inline Vec operator * (const T &v) const	
	{
		return Vec(x * v, y * v, z * v);
	}

	inline Vec operator / (const Vec& vx) const 
	{
		return Vec(x / vx.x, y / vx.y, z / vx.z); 
	};

	inline Vec operator / (const float &val) const 
	{
		return Vec(x / val, y / val, z / val); 
	};

	inline void operator -= (const Vec& vx)	
	{
		x -= vx.x;
		y -= vx.y;
		z -= vx.z;
	}
	inline void operator += (const Vec& vx)	
	{
		x += vx.x;
		y += vx.y;
		z += vx.z;
	}
	inline void operator /= (const Vec& vx)	
	{
		x /= vx.x;
		y /= vx.y;
		z /= vx.z;
	}
	inline void operator /= (const T &val) 
	{
		x /= val;
		y /= val;
		z /= val;
	}
	inline void operator *= (const Vec& vx)	
	{
		x *= vx.x;
		y *= vx.y;
		z *= vx.z;
	}
	inline void operator *= (const T &val) 
	{
		x *= val;
		y *= val;
		z *= val;
	}

	/*assignment*/ 
	inline void operator = (const Vec &vx)	
	{ 
		x = vx.x; 
		y = vx.y; 
		z = vx.z; 
	}
	inline void set(const T &nx, const T &ny, const T &nz)	
	{ 
		x = nx; 
		y = ny; 
		z = nz; 
	}
		
	inline void zero() 
	{ 
		x = 0; 
		y = 0; 
		z = 0; 
	}

	inline void invert() 
	{ 
		x = -x; 
		y = -y; 
		z = -z;
	}
	inline Vec inverse() const 
	{ 
		return Vec(-x, -y, -z); 
	}
	
	inline bool isZero() const 
	{ 
		return ((fabs((double)x) > PT_EPS || 
			fabs((double)y) > PT_EPS || 
			fabs((double)z) >PT_EPS) ? false : true); 
	}

	/*normalize*/ 
	inline bool unitize()
	{
		T dist = length();
		if (dist > PT_EPS)
		{
			x /= dist;
			y /= dist;
			z /= dist;
			return true;
		}
		return false;
	}
							
	/*length and distance*/ 
	inline T lengthSquared() const				
	{ 
		return x*x + y*y + z*z; 
	}
	inline T length() const					
	{ 
		return sqrt(lengthSquared()); 
	}
	inline T distance(const Vec &vx) const	
	{ 
		return sqrt(distanceSquared(vx));
	}

	inline T distanceSquared(const Vec &vx) const
	{
		T dx = x - vx.x;
		T dy = y - vx.y;
		T dz = z - vx.z;

		return dx*dx+dy*dy+dz*dz;
	}
	
	/*dot*/ 
	T dot (const Vec& vx) const 
	{ 
		return x*vx.x + y*vx.y + z*vx.z; 
	}

	/*cross*/ 
	inline Vec cross (const Vec& vx) const
	{
		return Vec(y*vx.z-z*vx.y,z*vx.x-x*vx.z, x*vx.y-y*vx.x);
	}
	inline Vec unitCross (const Vec& vx) const
	{
		Vec k(y*vx.z-z*vx.y,z*vx.x-x*vx.z, x*vx.y-y*vx.x);
		k.unitize();
		return k;
	}
};

//
// Point3 class
//
template <typename T>
class Point3
{
public:
	T x;
	T y;
	T z;

	typedef	Point3<T> Pnt;

	Point3<T>() : x(0), y(0), z(0) {};

	Point3<T>(const T*v)		
	{ 
		x = v[0]; y = v[1]; z = v[2]; 
	}
	Point3<T>(const T &nx, const T &ny, const T &nz)
	{
		x = nx; 
		y = ny;	
		z = nz;
	}

	/*copy constructor*/ 
	Pnt(const Pnt& v) 
	{ 
		*this = v; 
	}

	/*assignment */ 
	inline void operator = (const Pnt &vx)	
	{ 
		x = vx.x; 
		y = vx.y; 
		z = vx.z; 
	}
	inline Pnt operator + (const Vector3<T>& vx)	
	{
		Pnt p(vx.x+x, vx.y+y, vx.z+z);
		return p;
	}
	inline void set(const T &nx, const T &ny, const T &nz)	
	{ 
		x = nx; 
		y = ny; 
		z = nz; 
	}

	inline void operator -= (const Vector3<T>& vx)	
	{
		x -= vx.x;
		y -= vx.y;
		z -= vx.z;
	}
	inline void operator += (const Vector3<T>& vx)	
	{
		x += vx.x;
		y += vx.y;
		z += vx.z;
	}
};

template <typename T>
Point3<T> operator + (const Point3<T> &a, const Vector3<T> &b)
{
	return Point3<T>(a.x+b.x,a.y+b.y,a.z+b.z);
}	
template <typename T>
Vector3<T> operator + (const Vector3<T> &a, const Point3<T> &b)
{
	return Vector3<T>(a.x+b.x,a.y+b.y,a.z+b.z);
}

typedef Vector3<float>	Vector3f;
typedef Vector3<double>	Vector3d;

typedef Point3<float>	Point3f;
typedef Point3<double>	Point3d;

template <typename T>
class Cylinder
{
public:
	Cylinder() : radius(0), height(0) {};
	
	float		distanceToPnt( const Vector3<T> &pnt ) const
	{
		Vector3<T> p(pnt);
		p.x-=base.x;
		p.y-=base.y;
		p.z-=base.z;

		T t = axis.dot(p);

		Vector3<T> on_axis(axis * t + base);
		return fabs((on_axis - pnt).length() - radius);
	}

	Cylinder<T> &operator = ( const Cylinder<T> &c )
	{
		axis = c.axis;
		base = c.base;
		radius = c.radius;
		height = c.height;

		return *this;
	}
	Vector3<T>	axis;
	Point3<T>	base;
	T			radius;
	T			height;
};
typedef Cylinder<float>		Cylinderf;
typedef Cylinder<double>	Cylinderd;
}
#endif