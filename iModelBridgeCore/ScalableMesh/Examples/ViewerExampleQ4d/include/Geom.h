#pragma once

/*--------------------------------------------------------------------------*/
/* vector3																	*/
/*--------------------------------------------------------------------------*/
template <typename T>
class Vector3
{
public:
	T x;
	T y;
	T z;

	/*constructors*/
	Vector3() {};

	template<typename T2>
	Vector3(const Vector3<T2> &v) { x = v.x; y = v.y; z = v.z; };

	Vector3(const T*v) { x = v[0]; y = v[1]; z = v[2]; }

	template<typename T2>

	Vector3(const Vector3<T2> &a, const Vector3<T2> &b) { *this = b; *this -= Vector3d(a); };

	Vector3(const T &nx, const T &ny, const T &nz)
	{
		x = nx; y = ny;	z = nz;
	}
	/*copy constructor*/
	Vector3(const Vector3& vx) { *this = vx; };

	/*destructor*/
	~Vector3() {};

	/*accessors*/
	inline operator const T*() const { return ((const T*)this); };
	inline operator T *() { return ((T*)this); };

	/*comparative operators*/
	inline bool operator == (const Vector3<T>& vx) const
	{
		return (memcmp(&vx, this, sizeof(T) * 3) == 0 ? true : false);
	}
	inline float diff(const Vector3<T> &v) const
	{
		return fabs(v.x - x) + fabs(v.y - y) + fabs(v.z - z);
	}
	inline bool operator > (const Vector3<T>& vx) const
	{
		return ((x > vx.x
			&& y > vx.y
			&& z > vx.z)
			? true : false);
	}
	inline bool operator < (const Vector3<T>& vx) const
	{
		return ((x < vx.x
			&& y < vx.y
			&& z < vx.z)
			? true : false);
	}
	/*nearly equal - ie within tolerance*/
	bool equal(const Vector3<T>& vx, T tol) const
	{
		return (((vx.x > x - tol) && (vx.x < x + tol)
			&& (vx.y > y - tol) && (vx.y < y + tol)
			&& (vx.z > z - tol) && (vx.z < z + tol)
			) ? true : false);
	}
	inline Vector3 operator - () const {
		return Vector3<T>(-x, -y, -z);
	};

	inline Vector3 operator - (const Vector3<T>& vx) const {
		return Vector3<T>(x - vx.x, y - vx.y, z - vx.z);
	};

	inline Vector3 operator + (const Vector3<T>& vx) const {
		return Vector3<T>(x + vx.x, y + vx.y, z + vx.z);
	};

	inline Vector3 operator * (const Vector3<T>& vx) const {
		return Vector3<T>(x * vx.x, y * vx.y, z * vx.z);
	};

	inline Vector3 operator * (const float &v) const {
		return Vector3<T>(x * v, y * v, z * v);
	}

	inline Vector3<T> operator / (const Vector3<T>& vx) const {
		return Vector3<T>(x / vx.x, y / vx.y, z / vx.z);
	};

	inline Vector3<T> operator / (const float &val) const {
		return Vector3<T>(x / val, y / val, z / val);
	};

	inline void operator -= (const Vector3<T>& vx) {
		x -= vx.x;
		y -= vx.y;
		z -= vx.z;
	}
	inline void operator += (const Vector3<T>& vx) {
		x += vx.x;
		y += vx.y;
		z += vx.z;
	}
	inline void operator /= (const Vector3<T>& vx) {
		x /= vx.x;
		y /= vx.y;
		z /= vx.z;
	}
	inline void operator /= (const T &val) {
		x /= val;
		y /= val;
		z /= val;
	}
	inline void operator *= (const Vector3<T>& vx) {
		x *= vx.x;
		y *= vx.y;
		z *= vx.z;
	}
	inline void operator *= (const T &val) {
		x *= val;
		y *= val;
		z *= val;
	}

	/*assignment*/
	template <typename T2>
	inline void operator = (const Vector3<T2> &vx)	{ x = vx.x; y = vx.y; z = vx.z; };
	template <typename T2>
	inline void operator = (const Vector3<T2> *vx)	{ x = vx->x; y = vx->y; z = vx->z; };

	inline void get(T *d) const { d[0] = x; d[1] = y; d[2] = z; };
	inline void set(const T &nx, const T &ny, const T &nz) { x = nx; y = ny; z = nz; };
	inline void set(const T *d) { x = d[0]; y = d[1]; z = d[2]; };
	inline void set(const T &val) { x = val; y = val; z = val; };
	inline void zero() { x = 0; y = 0; z = 0; };

	inline void invert() { x = -x; y = -y; z = -z; }
	inline Vector3<T> inverse() const { return Vector3<T>(-x, -y, -z); }
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
	inline T length2() const { return x*x + y*y + z*z; }
	inline T length() const { return sqrt(length2()); }
	inline T dist(const Vector3<T> &vx) const { return sqrt(dist2(vx)); }

	inline T dist2(const Vector3<T> &vx) const
	{
		T dx = x - vx.x;
		T dy = y - vx.y;
		T dz = z - vx.z;

		return dx*dx + dy*dy + dz*dz;
	}
	/*dot*/
	T dot(const Vector3<T>& vx) const { return x*vx.x + y*vx.y + z*vx.z; }

	/*cross*/
	inline Vector3<T> cross(const Vector3<T>& vx) const
	{
		return Vector3<T>(y*vx.z - z*vx.y, z*vx.x - x*vx.z, x*vx.y - y*vx.x);
	}
	inline Vector3<T> unit_cross(const Vector3<T>& vx) const
	{
		Vector3<T> k(y*vx.z - z*vx.y, z*vx.x - x*vx.z, x*vx.y - y*vx.x);
		k.normalize();
		return k;
	}
	inline void between(const Vector3<T> &a, const Vector3<T> &b) { *this = b; *this -= a; };

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
		if (za == 1) { xa = 0; ya = 2; }
		else if (za == 2) { xa = 0; ya = 1; }
		else { xa = 1; ya = 2; }
	}
	bool is_finite() const { return isfinite(x) && isfinite(y) && isfinite(z); }
	bool is_finitef() const { return isfinitef(x) && isfinitef(y) && isfinitef(z); }

	bool is_nan() const { return _isnan(x) || _isnan(y) || _isnan(z); }
	bool is_nanf() const { return _isnanf(x) || _isnanf(y) || _isnanf(z); }

	inline static Vector3<T> &cast(T *v) { return *reinterpret_cast<Vector3<T>*>(v); }
	inline static const Vector3<T> &cast(const T *v) { return *reinterpret_cast<const Vector3<T>*>(v); }

	template <typename CastTo>
	void cast(Vector3<CastTo> &v) const { v.x = (CastTo)x; v.y = (CastTo)y; v.z = (CastTo)z; }
	
	inline	const	T	&operator()(const unsigned int i) const
	{
		return (this)(T*)[i];
	}
};

typedef Vector3<float>	Vector3f;
typedef Vector3<double>	Vector3d;

template <typename T>
struct Matrix4
{	
	Matrix4() {}

	Matrix4(const Matrix4 &m)
	{
		Matrix4(m_values, m.m_values, 16 * sizeof(T));
	}

	Matrix4(const Vector3<T> &xv, const Vector3<T> &yv, const Vector3<T> &zv)
	{
		setXVector(xv);
		setYVector(yv);
		setZVector(zv);
	}

	void zero()
	{
		memset(m_values, 0, 16 * sizeof(T));
	}

	void	identity()
	{
		for (unsigned int j = 0; j < 4; j++)
		{
			for (unsigned int i = 0; i < 4; i++)
			{
				(*this)(i, j) = (i == j) ? (T)1 : (T)0;
			}
		}
	}

	inline		Matrix4		&operator =(const Matrix4 &m)
	{
		if (&m != this) memcpy(m_values, m.m_values, 16 * sizeof(T));
		return *this;
	}

	// i down, j across
	inline	const	T	&operator()(const unsigned int i, const unsigned int j) const
	{
		return m_values[j*4 + i];
	}	

	inline	T &operator()(const unsigned int i, const unsigned int j)
	{
		return m_values[j*4 + i];
	}
	
	T		*values() const { return m_values; }

	inline		void	transpose()
	{		
		Matrix4<T> res;
		for (unsigned int j = 0; j < 4; j++)
		{
			for (unsigned int i = 0; i < 4; i++)
			{
				res(j, i) = (*this)(i, j);
			}
		}
		*this = res;
	}

	inline		void	setVector(int idx, const Vector3<T> &v)
	{
		for (unsigned int i = 0; i < 3; i++)
			(*this)(i, idx) = v(i);
	}

	inline		void	getVector(int idx, Vector3<T> &v) const
	{
		for (unsigned int i = 0; i < 3; i++)
			v(i) = (*this)(i, idx);
	}

	inline		void	setXVector(const Vector3<T> &v)
	{
		setVector(0, v);
	}
	inline		void	setYVector(const Vector3<T> &v)
	{
		setVector(1, v);
	}
	inline		void	setZVector(const Vector3<T> &v)
	{
		setVector(2, v);
	}

	inline		void	getXVector(Vector3<T> &v) const
	{
		getVector(0, v);
	}
	inline		void	getYVector(Vector3<T> &v) const
	{
		getVector(1, v);
	}
	inline		void	getZVector(Vector3<T> &v) const
	{
		getVector(2, v);
	}

	void	setTranslation(const Vector3<T> &v)
	{
		for (unsigned int i = 0; i < 3; i++)
		{
			(*this)(3, i) = v(i);
		}
	}

	void	translate(const Vector3<T> &v)
	{
		for (unsigned int i = 0; i < 3; i++)
		{
			(*this)(3, i) += v(i);
		}
	}

	/// Rotation about an axis.
	void axisAngle(const T angle, Vector3<T> a)
	{
		identity();

		// Rotations
		T radians = angle * 0.0174533;

		T s = sinf(radians);
		T c = cosf(radians);
		T t = 1.0F - c;

		(*this)(0,0) = t * a.x * a.x + c;
		(*this)(1,0) = t * a.x * a.y + s * a.z;
		(*this)(2,0) = t * a.x * a.z - s * a.y;
		(*this)(3,0) = 0.0F;

		(*this)(0,1) = t * a.y * a.x - s * a.z;
		(*this)(1,1) = t * a.y * a.y + c;
		(*this)(2,1) = t * a.y * a.z + s * a.x;
		(*this)(3,1) = 0.0F;

		(*this)(0,2) = t * a.z * a.x + s * a.y;
		(*this)(1,2) = t * a.z * a.y - s * a.x;
		(*this)(2,2) = t * a.z * a.z + c;
		(*this)(3,2) = 0.0F;

		(*this)(0,3) = 0.0F;
		(*this)(1,3) = 0.0F;
		(*this)(2,3) = 0.0F;
		(*this)(3,3) = 1.0F;

		transpose();
	}

	inline	T	determinant()
	{
		Matrix4<T>	&m = *this;
		return	  (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1)) * (m(2, 2) * m(3, 3) - m(3, 2) * m(2, 3))
			- (m(0, 0) * m(2, 1) - m(2, 0) * m(0, 1)) * (m(1, 2) * m(3, 3) - m(3, 2) * m(1, 3))
			+ (m(0, 0) * m(3, 1) - m(3, 0) * m(0, 1)) * (m(1, 2) * m(2, 3) - m(2, 2) * m(1, 3))
			+ (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1)) * (m(0, 2) * m(3, 3) - m(3, 2) * m(0, 3))
			- (m(1, 0) * m(3, 1) - m(3, 0) * m(1, 1)) * (m(0, 2) * m(2, 3) - m(2, 2) * m(0, 3))
			+ (m(2, 0) * m(3, 1) - m(3, 0) * m(2, 1)) * (m(0, 2) * m(1, 3) - m(1, 2) * m(0, 3));
	}

	void	invert()
	{
		T	d = determinant();
		if (d == 0.0) return;

		d = 1.0 / d;

		Matrix4<T>	&m = *this;
		Matrix4<T>	result;

		result(0, 0) = d * (m(1, 1) * (m(2, 2) * m(3, 3) - m(3, 2) * m(2, 3)) + m(2, 1) * (m(3, 2) * m(1, 3) - m(1, 2) * m(3, 3)) + m(3, 1) * (m(1, 2) * m(2, 3) - m(2, 2) * m(1, 3)));
		result(1, 0) = d * (m(1, 2) * (m(2, 0) * m(3, 3) - m(3, 0) * m(2, 3)) + m(2, 2) * (m(3, 0) * m(1, 3) - m(1, 0) * m(3, 3)) + m(3, 2) * (m(1, 0) * m(2, 3) - m(2, 0) * m(1, 3)));
		result(2, 0) = d * (m(1, 3) * (m(2, 0) * m(3, 1) - m(3, 0) * m(2, 1)) + m(2, 3) * (m(3, 0) * m(1, 1) - m(1, 0) * m(3, 1)) + m(3, 3) * (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1)));
		result(3, 0) = d * (m(1, 0) * (m(3, 1) * m(2, 2) - m(2, 1) * m(3, 2)) + m(2, 0) * (m(1, 1) * m(3, 2) - m(3, 1) * m(1, 2)) + m(3, 0) * (m(2, 1) * m(1, 2) - m(1, 1) * m(2, 2)));
		result(0, 1) = d * (m(2, 1) * (m(0, 2) * m(3, 3) - m(3, 2) * m(0, 3)) + m(3, 1) * (m(2, 2) * m(0, 3) - m(0, 2) * m(2, 3)) + m(0, 1) * (m(3, 2) * m(2, 3) - m(2, 2) * m(3, 3)));
		result(1, 1) = d * (m(2, 2) * (m(0, 0) * m(3, 3) - m(3, 0) * m(0, 3)) + m(3, 2) * (m(2, 0) * m(0, 3) - m(0, 0) * m(2, 3)) + m(0, 2) * (m(3, 0) * m(2, 3) - m(2, 0) * m(3, 3)));
		result(2, 1) = d * (m(2, 3) * (m(0, 0) * m(3, 1) - m(3, 0) * m(0, 1)) + m(3, 3) * (m(2, 0) * m(0, 1) - m(0, 0) * m(2, 1)) + m(0, 3) * (m(3, 0) * m(2, 1) - m(2, 0) * m(3, 1)));
		result(3, 1) = d * (m(2, 0) * (m(3, 1) * m(0, 2) - m(0, 1) * m(3, 2)) + m(3, 0) * (m(0, 1) * m(2, 2) - m(2, 1) * m(0, 2)) + m(0, 0) * (m(2, 1) * m(3, 2) - m(3, 1) * m(2, 2)));
		result(0, 2) = d * (m(3, 1) * (m(0, 2) * m(1, 3) - m(1, 2) * m(0, 3)) + m(0, 1) * (m(1, 2) * m(3, 3) - m(3, 2) * m(1, 3)) + m(1, 1) * (m(3, 2) * m(0, 3) - m(0, 2) * m(3, 3)));
		result(1, 2) = d * (m(3, 2) * (m(0, 0) * m(1, 3) - m(1, 0) * m(0, 3)) + m(0, 2) * (m(1, 0) * m(3, 3) - m(3, 0) * m(1, 3)) + m(1, 2) * (m(3, 0) * m(0, 3) - m(0, 0) * m(3, 3)));
		result(2, 2) = d * (m(3, 3) * (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1)) + m(0, 3) * (m(1, 0) * m(3, 1) - m(3, 0) * m(1, 1)) + m(1, 3) * (m(3, 0) * m(0, 1) - m(0, 0) * m(3, 1)));
		result(3, 2) = d * (m(3, 0) * (m(1, 1) * m(0, 2) - m(0, 1) * m(1, 2)) + m(0, 0) * (m(3, 1) * m(1, 2) - m(1, 1) * m(3, 2)) + m(1, 0) * (m(0, 1) * m(3, 2) - m(3, 1) * m(0, 2)));
		result(0, 3) = d * (m(0, 1) * (m(2, 2) * m(1, 3) - m(1, 2) * m(2, 3)) + m(1, 1) * (m(0, 2) * m(2, 3) - m(2, 2) * m(0, 3)) + m(2, 1) * (m(1, 2) * m(0, 3) - m(0, 2) * m(1, 3)));
		result(1, 3) = d * (m(0, 2) * (m(2, 0) * m(1, 3) - m(1, 0) * m(2, 3)) + m(1, 2) * (m(0, 0) * m(2, 3) - m(2, 0) * m(0, 3)) + m(2, 2) * (m(1, 0) * m(0, 3) - m(0, 0) * m(1, 3)));
		result(2, 3) = d * (m(0, 3) * (m(2, 0) * m(1, 1) - m(1, 0) * m(2, 1)) + m(1, 3) * (m(0, 0) * m(2, 1) - m(2, 0) * m(0, 1)) + m(2, 3) * (m(1, 0) * m(0, 1) - m(0, 0) * m(1, 1)));
		result(3, 3) = d * (m(0, 0) * (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) + m(1, 0) * (m(2, 1) * m(0, 2) - m(0, 1) * m(2, 2)) + m(2, 0) * (m(0, 1) * m(1, 2) - m(1, 1) * m(0, 2)));
		*this = result;
	}



private:
	T m_values[16];
};

template<typename T>
Vector3<T>	operator *(const Vector3<T> v, const struct Matrix4<T> &m)
{
	Vector3<T> r;
	r.x = m(0, 0) * v.x
		+ m(1, 0) * v.y
		+ m(2, 0) * v.z
		+ m(3, 0);

	r.y = m(0, 1) * v.x
		+ m(1, 1) * v.y
		+ m(2, 1) * v.z
		+ m(3, 1);

	r.z = m(0, 2) * v.x
		+ m(1, 2) * v.y
		+ m(2, 2) * v.z
		+ m(3, 2);

	return r;
}

template<typename T>
inline void operator *=(Vector3<T> &v, const struct Matrix4<T> &m)
{
	Vector3<T> r;
	r.x = m(0, 0) * v.x
		+ m(1, 0) * v.y
		+ m(2, 0) * v.z
		+ m(3, 0);

	r.y = m(0, 1) * v.x
		+ m(1, 1) * v.y
		+ m(2, 1) * v.z
		+ m(3, 1);

	r.z = m(0, 2) * v.x
		+ m(1, 2) * v.y
		+ m(2, 2) * v.z
		+ m(3, 2);

	v = r;
}

typedef Matrix4<float> Matrix4f;
typedef Matrix4<double> Matrix4d;

