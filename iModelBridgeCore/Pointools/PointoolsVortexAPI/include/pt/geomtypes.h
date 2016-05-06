/*--------------------------------------------------------------------------*/ 
/*  geomTypes.h																*/ 
/*	basic geometry types definition + implementation						*/ 
/*  Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.  */
/*																			*/ 
/*  Last Updated 12 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#ifndef COMMONCLASSES_GEOMTYPES
#define COMMONCLASSES_GEOMTYPES

#ifdef __INTEL_COMPILER
#include <mathimf.h>
#else
#include <math.h>
#endif

#include <memory.h>
#include <pt/classes.h>

namespace pt
{
/*--------------------------------------------------------------------------*/ 
/* vector2																	*/ 
/*--------------------------------------------------------------------------*/ 
template <class T>
class vec2
{
public:
	T x;
	T y;

	/*constructors*/ 
	vec2<T>(const T &nx, const T &ny)
	{ x = nx; y = ny; }
	/*default*/ 
	vec2<T>() {};
	vec2<T>(const vec2<T> &a, const vec2<T> &b) { *this = b; *this -= a; };
	/*copy constructor*/ 
	vec2<T>(const vec2<T>& v) { *this = v; };
	/*destructor*/ 
	~vec2<T>() {};

	/*accessors*/ 
	inline operator const T*() const { return ((const T*)this); };
	inline operator T *() { return ((T*)this); };

	/*comparative operators*/ 
	inline bool operator == (const vec2<T>& v) const
	{
		return (memcmp(&v, this, sizeof(T)*2)== 0 ? true : false);
	}
	inline bool operator > (const vec2<T>& v) const
	{
		return ((x > v.x
			&& y > v.y)
			? true : false);
	}
	inline bool operator < (const vec2<T>& v) const
	{
			return ((x < v.x
					&& y < v.y)
					? true : false);
	}
	/*nearly equal - ie within tolerance*/ 
	bool equal(const vec2<T>& v, T tol) const
	{
		return ((fabs(v.x - x) <= tol)	
				&& (fabs(v.y - y) <= tol)) ? true : false;
	}

	inline vec2<T> operator - (const vec2<T>& v) const {
		return vec2<T>(x - v.x, y - v.y); };

	inline vec2<T> operator + (const vec2<T>& v) const {
		return vec2<T>(x + v.x, y + v.y); };

	inline vec2<T> operator * (const vec2<T>& v) const {
		return vec2<T>(x * v.x, y * v.y); };

	inline vec2<T> operator * (const T &v) const	{
		return vec2<T>(x * v, y * v);}

	inline vec2<T> operator * (T v) const	{
		return vec2<T>(x * v, y * v);}

	inline vec2<T> operator / (const vec2<T>& v) const {
		return vec2<T>(x / v.x, y / v.y); };

	inline vec2<T> operator / (T val) const {
		return vec2<T>(x / val, y / val); };

	inline vec2<T> operator / (const T &val) const {
		return vec2<T>(x / val, y / val); };

	inline void operator -= (const vec2<T>& v)	{
			x -= v.x;
			y -= v.y;
		}
	inline void operator += (const vec2<T>& v)	{
			x += v.x;
			y += v.y;
		}
	inline void operator /= (const vec2<T>& v)	{
			x /= v.x;
			y /= v.y;
		}
	inline void operator /= (const T &val) {
			x /= val;
			y /= val;
		}
	inline void operator *= (const vec2<T>& v)	{
			x *= v.x;
			y *= v.y;
		}
	inline void operator *= (const T &val) {
			x *= val;
			y *= val;
		}
	inline void operator *= (T val)  {
			x *= val;
			y *= val;
		}
	/*assignment*/ 
	inline void operator = (const vec2<T> &v)	{ x = v.x; y = v.y;};
	inline void operator = (const vec2<T> *v)	{ x = v->x; y = v->y;};

	inline void set(const T &nx, const T &ny)	{ x = nx; y = ny; };
	inline void set(const T d[2])	{ x = d[0]; y = d[1]; };
	inline void set(const T &val)	{ x = val; y = val; };
	inline void zero()					{ x = 0; y = 0;  };

	inline void invert() { x = -x; y = -y;}
	inline vec2<T> inverse() const { return vec2<T>(-x, -y); }

	inline bool is_zero() {
		return ((fabs(x) > 0 ||
				 fabs(y) > 0) ? false : true); };

	inline void get(T d[2]) const { memcpy(d, this, sizeof(T) *2); };

	/*normalize*/ 
	inline T length2() const	{ return x*x + y*y; }
	inline T length() const	{ return (T)sqrt(length2()); }
	inline void normalize()
	{
		T dist = length();
		if (dist)
		{
			x /= dist;
			y /= dist;
		}
	}
	inline T dist(const vec2<T> &v)	const	{ return (T)sqrt(dist2(v)); }
	inline T dist2(const vec2<T> &v) const
	{
		T dx = x - v.x;
		T dy = y - v.y;
		return dx*dx+dy*dy;
	}
	/*dot*/ 
	inline T dot (const vec2<T>& v) const {	return x*v.x + y*v.y; }

	void between(const vec2<T> &a, const vec2<T> &b) { *this = b; *this -= a; };
	void trim(const T d)
	{
		T l = length();
		double c = d / l;
		c = 1- c;
		x *= c;
		y *= c;

	}
	void extend(const T d)
	{
		T l = length();
		double c = d / l;
		c += 1;
		x *= c;
		y *= c;
	}
};
/*--------------------------------------------------------------------------*/ 
/* vector3	s (short)														*/ 
/*--------------------------------------------------------------------------*/ 
class vector3s
{
public:
	short x;
	short y;
	short z;
	
	/*constructors*/ 
	vector3s() {};
	vector3s(const short*v) { x = v[0]; y = v[1]; z = v[2]; }
	vector3s(const short &a, const short &b,const short &c) { x = a; y = b; z = c; }

	/*copy constructor*/ 
	vector3s(const vector3s& vx) { *this = vx; };

	/*destructor*/ 
	~vector3s() {};

	/*accessors*/ 
	inline operator const short*() const { return ((const short*)this); };

	/*comparative operators*/ 
	inline bool operator == (const vector3s& vx) const
	{
		return (memcmp(&vx, this, sizeof(short)*3)== 0 ? true : false);
	}
	inline float diff(const vector3s &v) const
	{
		return fabs((float)(v.x - x)) + fabs((float)(v.y - y)) + fabs((float)(v.z - z));
	}

	inline vector3s operator - () const {
		return vector3s(-x, -y, -z); };

	inline vector3s operator - (const vector3s& vx) const {
		return vector3s(x - vx.x, y - vx.y, z - vx.z); };

	inline vector3s operator + (const vector3s& vx) const {
		return vector3s(x + vx.x, y + vx.y, z + vx.z); };

	inline vector3s operator * (const vector3s& vx) const {
		return vector3s(x * vx.x, y * vx.y, z * vx.z); };

	inline vector3s operator * (const short &v) const	{
		return vector3s(x * v, y * v, z * v);}

	inline vector3s operator / (const vector3s& vx) const {
		return vector3s(x / vx.x, y / vx.y, z / vx.z); };

	inline vector3s operator / (const short &val) const {
		return vector3s(x / val, y / val, z / val); };

	inline void operator -= (const vector3s& vx)	{
			x -= vx.x;
			y -= vx.y;
			z -= vx.z;
		}
	inline void operator += (const vector3s& vx)	{
			x += vx.x;
			y += vx.y;
			z += vx.z;
		}
	inline void operator /= (const vector3s& vx)	{
			x /= vx.x;
			y /= vx.y;
			z /= vx.z;
		}
	inline void operator /= (const float &val) {
			x = static_cast<short>(x / val);
            y = static_cast<short>(y / val);
            z = static_cast<short>(z / val);
		}
	inline void operator /= (const int &val) {
			x /= val;
			y /= val;
			z /= val;
		}
	inline void operator *= (const vector3s& vx)	{
			x *= vx.x;
			y *= vx.y;
			z *= vx.z;
		}
	inline void operator *= (const float &val) {
        x = static_cast<short>(x * val);
        y = static_cast<short>(y * val);
        z = static_cast<short>(z * val);
    }
	inline void operator *= (const int &val) {
			x *= val;
			y *= val;
			z *= val;
		}

	/*assignment*/ 
	inline void operator = (const vector3s &vx)	{ x = vx.x; y = vx.y; z = vx.z; };
	inline void operator = (const vector3s *vx)	{ x = vx->x; y = vx->y; z = vx->z; };

	inline void set(const short *d)	{ x = d[0]; y = d[1]; z = d[2]; };
	inline void zero() { x = 0; y = 0; z = 0; };

	inline void invert() { x = -x; y = -y; z = -z;}
	inline vector3s inverse() const { return vector3s(-x, -y, -z); }
	inline bool is_zero() const { return (x == 0 && y == 0 && z == 0) ? false : true; };

	/*normalize*/ 
	inline void normalize()
	{
		float dist = length();
		if (dist)
		{
            x = static_cast<short>(x / dist);
            y = static_cast<short>(y / dist);
            z = static_cast<short>(z / dist);

		}
	}
	/*length and distance*/ 
	inline float length2() const				{ return static_cast<float>(x*x + y*y + z*z); }
	inline float length() const					{ return sqrt(length2()); }
	inline float dist(const vector3s &vx) const	{ return sqrt(dist2(vx));}
	
	inline float dist2(const vector3s &vx) const
	{
		float dx = (float)(x - vx.x);
		float dy = (float)(y - vx.y);
		float dz = (float)(z - vx.z);

		return dx*dx+dy*dy+dz*dz;
	}
	/*dot*/ 
	float dot (const vector3s& vx) const { return (float)(x*vx.x + y*vx.y + z*vx.z); }
	
	/*cross*/ 
	inline vector3s cross (const vector3s& vx) const
	{
		return vector3s(y*vx.z-z*vx.y,z*vx.x-x*vx.z, x*vx.y-y*vx.x);
	}
	inline vector3s unit_cross (const vector3s& vx) const
	{
		vector3s k(y*vx.z-z*vx.y,z*vx.x-x*vx.z, x*vx.y-y*vx.x);
		k.normalize();
		return k;
	}
	inline void between(const vector3s &a, const vector3s &b) { *this = b; *this -= a; };

	int major_axis() const
	{
		int axis = 0;
#ifdef __INTEL_COMPILER
		if (fabs(y) >= fabs(x) && fabs(y) >= fabs(z)) axis = 1;
		else if (fabs(z) >= fabs(x) && fabs(z) >= fabs(y)) axis = 2;
#else
		if (abs(y) >= abs(x) && abs(y) >= abs(z)) axis = 1;
		else if (abs(z) >= abs(x) && abs(z) >= abs(y)) axis = 2;
#endif
		return axis;
	}
	void dominant_axes(int &xa, int &ya, int &za) const
	{
		za = major_axis();
		if (za==1)		{ xa = 0; ya = 2; }
		else if (za==2)	{ xa = 0; ya = 1; }
		else {	xa = 1; ya = 2; }
	}
};
/*--------------------------------------------------------------------------*/ 
/* vector3																	*/ 
/*--------------------------------------------------------------------------*/ 
template <typename T>
class vec3
{
public:
	T x;
	T y;
	T z;
	
	/*constructors*/ 
	vec3() {};
	vec3(const vector3s &vs) { x = vs.x; y = vs.y; z = vs.z; }
	vec3(const float*v) { x = v[0]; y = v[1]; z = v[2]; }
	vec3(const double*v) { x = static_cast<T>(v[0]); y = static_cast<T>(v[1]); z = static_cast<T>(v[2]); }
	vec3(const vec3<T> &a, const vec3<T> &b) { *this = b; *this -= a; };
	vec3(const T &nx, const T &ny, const T &nz)
	{
		x = nx; y = ny;	z = nz;
	}
	/*copy constructor*/ 
	vec3(const vec3& vx) { *this = vx; };

	/*destructor*/ 
	~vec3() {};

	/*accessors*/ 
	inline operator const T*() const { return ((const T*)this); };
	inline operator T *() { return ((T*)this); };

	/*comparative operators*/ 
	inline bool operator == (const vec3& vx) const
	{
		return (memcmp(&vx, this, sizeof(T)*3)== 0 ? true : false);
	}
	inline float diff(const vec3<T> &v) const
	{
		return fabs(v.x - x) + fabs(v.y - y) + fabs(v.z - z);
	}
	inline bool operator > (const vec3& vx) const
	{
		return ((x > vx.x
			&& y > vx.y
			&& z > vx.z)
			? true : false);
	}
	inline bool operator < (const vec3& vx) const
		{
			return ((x < vx.x
					&& y < vx.y
					&& z < vx.z)
					? true : false);
	}
	/*nearly equal - ie within tolerance*/ 
	bool equal(const vec3& vx, float tol) const
	{
		return ((	(vx.x > x-tol) && (vx.x < x+tol)
				&&	(vx.y > y-tol) && (vx.y < y+tol)
				&&	(vx.z > z-tol) && (vx.z < z+tol)
				) ? true : false);
	}
	inline vec3 operator - () const {
		return vec3<T>(-x, -y, -z); };

	inline vec3 operator - (const vec3<T>& vx) const {
		return vec3<T>(x - vx.x, y - vx.y, z - vx.z); };

	inline vec3 operator + (const vec3<T>& vx) const {
		return vec3<T>(x + vx.x, y + vx.y, z + vx.z); };

	inline vec3 operator * (const vec3<T>& vx) const {
		return vec3<T>(x * vx.x, y * vx.y, z * vx.z); };

	inline vec3 operator * (const float &v) const	{
		return vec3<T>(x * v, y * v, z * v);}

	inline vec3 operator * (const double &v) const	{
		return vec3<T>(x * v, y * v, z * v);}

	inline vec3<T> operator / (const vec3<T>& vx) const {
		return vec3<T>(x / vx.x, y / vx.y, z / vx.z); };

	inline vec3<T> operator / (const float &val) const {
		return vec3<T>(x / val, y / val, z / val); };

	inline void operator -= (const vec3<T>& vx)	{
			x -= vx.x;
			y -= vx.y;
			z -= vx.z;
		}
	inline void operator += (const vec3<T>& vx)	{
			x += vx.x;
			y += vx.y;
			z += vx.z;
		}
	inline void operator /= (const vec3<T>& vx)	{
			x /= vx.x;
			y /= vx.y;
			z /= vx.z;
		}
	inline void operator /= (const float &val) {
			x /= val;
			y /= val;
			z /= val;
		}
	inline void operator /= (const double &val) {
			x /= static_cast<T>(val);
			y /= static_cast<T>(val);
			z /= static_cast<T>(val);
		}
	inline void operator /= (const int &val) {
			x /= val;
			y /= val;
			z /= val;
		}
	inline void operator *= (const vec3<T>& vx)	{
			x *= vx.x;
			y *= vx.y;
			z *= vx.z;
		}
	inline void operator *= (const float &val) {
			x *= val;
			y *= val;
			z *= val;
		}
	inline void operator *= (const double &val) {
			x *= val;
			y *= val;
			z *= val;
		}
	inline void operator *= (const int &val) {
			x *= val;
			y *= val;
			z *= val;
		}

	/*assignment*/ 
	inline void operator = (const vec3<T> &vx)	{ x = vx.x; y = vx.y; z = vx.z; };
	inline void operator = (const vector3s &vx)	{ x = vx.x; y = vx.y; z = vx.z; };
	inline void operator = (const vec3<T> *vx)	{ x = vx->x; y = vx->y; z = vx->z; };

	inline void get(double *d) const { d[0] = x; d[1] = y; d[2] = z; };
	inline void get(float *d) const { d[0] = x; d[1] = y; d[2] = z; };
	inline void set(const float &nx, const float &ny, const float &nz)	{ x = nx; y = ny; z = nz; };
	inline void set(const float *d)	{ x = d[0]; y = d[1]; z = d[2]; };
	inline void set(const double *d)	{ x = static_cast<T>(d[0]); y = static_cast<T>(d[1]); z = static_cast<T>(d[2]); };
	inline void set(const float &val) { x = val; y = val; z = val; };
	inline void zero() { x = 0; y = 0; z = 0; };

	inline void invert() { x = -x; y = -y; z = -z;}
	inline vec3<T> inverse() const { return vec3<T>(-x, -y, -z); }
	inline bool is_zero() const { return ((fabs((double)x) > 0.000001f || fabs((double)y) > 0.000001f || fabs((double)z) > 0.000001f) ? false : true); };

	/*normalize*/ 
	inline void normalize()
	{
		T dist = length();
		if (dist)
		{
			x /= dist;
			y /= dist;
			z /= dist;
		}
	}
	/*length and distance*/ 
	inline T length2() const				{ return x*x + y*y + z*z; }
	inline T length() const					{ return sqrt(length2()); }
	inline T dist(const vec3<T> &vx) const	{ return sqrt(dist2(vx));}
	
	inline T dist2(const vec3<T> &vx) const
	{
		T dx = x - vx.x;
		T dy = y - vx.y;
		T dz = z - vx.z;

		return dx*dx+dy*dy+dz*dz;
	}
	/*dot*/ 
	T dot (const vec3<T>& vx) const { return x*vx.x + y*vx.y + z*vx.z; }
	
	/*cross*/ 
	inline vec3<T> cross (const vec3<T>& vx) const
	{
		return vec3<T>(y*vx.z-z*vx.y,z*vx.x-x*vx.z, x*vx.y-y*vx.x);
	}
	inline vec3<T> unit_cross (const vec3<T>& vx) const
	{
		vec3<T> k(y*vx.z-z*vx.y,z*vx.x-x*vx.z, x*vx.y-y*vx.x);
		k.normalize();
		return k;
	}
	inline void between(const vec3<T> &a, const vec3<T> &b) { *this = b; *this -= a; };

	int major_axis() const
	{
		int axis = 0;
		if (fabs(y) >= fabs(x) && fabs(y) >= fabs(z)) axis = 1;
		else if (fabs(z) >= fabs(x) && fabs(z) >= fabs(y)) axis = 2;
		return axis;
	}
	void dominant_axes(int &xa, int &ya, int &za) const
	{
		za = major_axis();
		if (za==1)		{ xa = 0; ya = 2; }
		else if (za==2)	{ xa = 0; ya = 1; }
		else {	xa = 1; ya = 2; }
	}
	bool is_finite() const  { return isfinite(x) && isfinite(y) && isfinite(z);		}
	bool is_finitef() const { return isfinitef(x) && isfinitef(y) && isfinitef(z);	}	

	bool is_nan() const { return _isnan(x) || _isnan(y) || _isnan(z); }
	bool is_nanf() const { return _isnanf(x) || _isnanf(y) || _isnanf(z); }

	inline static vec3<T> &cast(T *v)  { return *reinterpret_cast<vec3<T>*>(v); }
	inline static const vec3<T> &cast(const T *v) { return *reinterpret_cast<const vec3<T>*>(v); }

	template <typename CastTo>
	void cast(vec3<CastTo> &v) const { v.x = (CastTo)x; v.y = (CastTo)y; v.z = (CastTo)z; }
};

typedef vec3<float>		vector3;
typedef vec3<double>	vector3d;
typedef vec3<int>		vector3i;

typedef vec2<float>		vector2;
typedef vec2<int>		vector2i;

class vector4d
{
public:
	double x;
	double y;
	double z;
	double w;

	vector4d() {};
	vector4d(vector3 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = 1.0;
	}
	vector4d(vector3d &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = 1.0;
	}
	vector4d(const double &_x, const double &_y, const double &_z, const double &_w) 
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}
	inline double dot (const vector4d& vx) const { return x*vx.x + y*vx.y + z*vx.z + w*vx.w; }	

	inline void operator -= (const vector3& vx)	{
			x -= vx.x;
			y -= vx.y;
			z -= vx.z;
		}
	inline void operator += (const vector3& vx)	{
			x += vx.x;
			y += vx.y;
			z += vx.z;
		}
	inline void operator /= (const vector3& vx)	{
			x /= vx.x;
			y /= vx.y;
			z /= vx.z;
		}

	inline void operator -= (const vector4d& vx)	{
			x -= vx.x;
			y -= vx.y;
			z -= vx.z;
			w -= vx.w;
		}
	inline void operator += (const vector4d& vx)	{
			x += vx.x;
			y += vx.y;
			z += vx.z;
			w += vx.w;
		}
	inline void operator /= (const vector4d& vx)	{
			x /= vx.x;
			y /= vx.y;
			z /= vx.z;
			w /= vx.w;
		}
	inline void operator *= (const vector4d& vx)	{
			x *= vx.x;
			y *= vx.y;
			z *= vx.z;
			w *= vx.w;
		}
	inline void operator *= (const vector3& vx)	{
			x *= vx.x;
			y *= vx.y;
			z *= vx.z;
		}
	inline void operator /= (const double &val) {
			x /= val;
			y /= val;
			z /= val;
		}

	inline void operator /= (const float &val) {
			x /= val;
			y /= val;
			z /= val;
		}
	inline void operator /= (const int &val) {
			x /= val;
			y /= val;
			z /= val;
		}
	inline void operator *= (const float &val) {
			x *= val;
			y *= val;
			z *= val;
		}
	inline void operator *= (const int &val) {
			x *= val;
			y *= val;
			z *= val;
		}
	/*accessors*/ 
	inline operator const double*() const { return ((const double*)this); };
	inline operator double *() { return ((double*)this); };
};

template <typename Real>
struct Ray
{
	Ray(const Ray<Real> &copy) { (*this) = copy; }
	Ray(const vec3<Real> &_origin, const vec3<Real> &_dir)
		: origin(_origin), direction(_dir)	{}
	
	Ray(){};

	Ray<Real> &operator = (const Ray<Real> &copy)
	{
		direction = copy.direction;
		origin = copy.origin;
		return *this;
	}
	bool operator == (const Ray<Real> &copy) const
	{
		return (direction == copy.direction && origin == copy.origin)
			? true : false;
	}
	void normalise() 
	{ 
		direction.normalize(); 
	}	
	vec3<Real> pointAt(Real t) const
	{
		return origin + direction * t;
	}
	bool perpDist2Pnt(const vec3<Real> &pnt, double &dist, double &t) const
	{	
		perpPntProjection(pnt, t);

        vec3<Real> onRay(origin + direction * static_cast<Real>(t));
		dist = (onRay - pnt).length();

		return (t >= 0) ? true : false;
	}
	bool perpDistSq2Pnt(const vec3<Real> &pnt, double &dist, double &t) const
	{	
		perpPntProjection(pnt, t);

		vec3<Real> onRay(origin + direction * t);
		dist = (onRay - pnt).length2();

		return (t >= 0) ? true : false;
	}
	void perpPntProjection(const vec3<Real> &pnt, double &t) const
	{	
		t = direction.dot(pnt - origin);
	}
#if defined(NEEDS_WORK_VORTEX_DGNDB)
    Invalid use of members in static function
	template <typename CastTo>
	static void cast(Ray<CastTo> &ray)
	{
		direction.cast(ray.direction);
		origin.cast(ray.origin);
	}
#endif
	vec3<Real> origin;
	vec3<Real> direction;
};
template <typename Real>
struct Segment
{
	Segment(const vec3<Real> &p0, const vec3<Real> &p1)
		: a(p0), b(p1)	{}
	Segment() {};

#if defined(NEEDS_WORK_VORTEX_DGNDB)
p0 not defined in scope
	Ray<Real> ray() 
	{ 
		vec3<Real> dir(p0,p1);
		dir.normalize();

		return Ray<Real>(p0, dir); 
	}	
#endif
	vec3<Real> pointAt(Real t) const
	{
		vec3<Real> v(a,b);
		v.normalize();

		return a + v * t;
	}
#if defined(NEEDS_WORK_VORTEX_DGNDB)
origin not defined in scope
	bool perpDist2Pnt(const vec3<Real> &pnt, Real &dist, Real &t) const
	{	
		double b = perpPntProjection(pnt);
		b = t;

		pt::vector3 Pb = origin + direction * b;
		return pnt.dot(Pb);
	}
	bool perpPntProjection(const vec3<Real> &pnt, Real &t) const
	{	
		pt::vec3<Real> w(origin, pnt);

		double c1 = w.dot( direction );
		if (c1 < 0) return false;

		double c2 = direction.dot( direction );
		if (c2 <= c1) return false;

		if (fabs(c2) > 0.000001f)
		{
			t = (Real)(c1 / c2);
			return true;
		}
		return false;	
	}
#endif
	vec3<Real> a;
	vec3<Real> b;
};
typedef Ray<float> Rayf;
typedef Ray<double> Rayd;
typedef Segment<float> Segmentf;
typedef Segment<double> Segementd;
}
#endif
