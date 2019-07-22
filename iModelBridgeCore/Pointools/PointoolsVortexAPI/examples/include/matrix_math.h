// ---------------------------------------------------------------------------------------------------------------------------------
//  _     _ __  __       _   _         _
// | |   | |  \/  |     | | | |       | |
// | |   | | \  / | __ _| |_| |__     | |__
//  \ \ / /| |\/| |/ _` | __| '_ \    | '_ \
//   \ V / | |  | | (_| | |_| | | | _ | | | |
//    \_/  |_|  |_|\__,_|\__|_| |_|(_)|_| |_|
//
//
//
// Generic 2-dimensional NxM matrix/vector mathematics class specialized for 3D usage
//
// Best viewed with 8-character tabs and (at least) 132 columns
//
// ---------------------------------------------------------------------------------------------------------------------------------
//
// Restrictions & freedoms pertaining to usage and redistribution of this software:
//
//  * This software is 100% free
//  * If you use this software (in part or in whole) you must credit the author.
//  * This software may not be re-distributed (in part or in whole) in a modified
//    form without clear documentation on how to obtain a copy of the original work.
//  * You may not use this software to directly or indirectly cause harm to others.
//  * This software is provided as-is and without warrantee. Use at your own risk.
//
// For more information, visit HTTP://www.FluidStudios.com
//
// ---------------------------------------------------------------------------------------------------------------------------------
// Originally created on 12/06/2000 by Paul Nettle
//
// Copyright 2000, Fluid Studios, Inc., all rights reserved.
// ---------------------------------------------------------------------------------------------------------------------------------
//
// IMPORTANT NOTE!
//
// Due to the nature of this class being a single solution for all 2D matrices of NxM size (which includes matrices, vectors &
// points) there are two cases where the class's behavior does not make as much sense as you might like. These two cases are in
// reference to the >> and ^ operators.
//
// A multiplication operator is commonly used for component-wise multiplication, dot products and matrix concatenation. Since the
// class supports the operators [+, -, /, *, +=, -=, /=, *=] it was prudent to maintain consistency and let operator* and
// operator *= work as component-wise multiplication. So what operators should be used for dot products and concatenation? I have
// chosen >> for concatenation, because it helps to serve as a reminder in which direction the operation's associativity goes (from
// left-to-right). The dot product uses operator ^ (simply for lack of a better operator.)
//
// The problem you may find then, is that operator >> and operator ^ have lower precedence than addition & subtraction. Without the
// use of parenthesis around these multiplicative operations, you will not get the result you expect.
//
// Furthermore, operator ^ (dot product) has a lower precedence than even operators [<, <=, >, >=]. This can cause prolems in cases
// like:  "if (v1 ^ v2 > 0)".
//
// If you do not like the operators I have chosen, feel free to modify them. Or you may chose to perform these operations through
// the function calls dot(), concat(), cross(), etc.
//
// Consider yourself warned.
//
// ---------------------------------------------------------------------------------------------------------------------------------

#ifndef	_H_VMATH
#define _H_VMATH

// --------------------------------------------------------------------------------------------------------------------------------
// Often times, it's better to use the compiler to generate an error rather than a runtime assert. The following metaprogram (if
// you want to call it that) provides nice compile-time errors on a false condition.
//
// Note that the compiler will probably complain about a funciton that doesn't exist, which is fine. The fact still remains that
// the compiler DOES generate an error that points to the line containing the TemplateAssert macro call.
//
// Also note that much like runtime asserts that get conditionally compiled out during non-debug builds, this gets 'optimized' out
// always (provided it doesn't generate a compiler error.)
// --------------------------------------------------------------------------------------------------------------------------------
#include <math.h>
#include <gl/gl.h>

#ifdef __INTEL_COMPILER
template <bool B> struct templateAssert {};
struct templateAssert<true> {static void there_has_been_a_compile_time_assertion() {};};

#define	TemplateAssert(__a) {const bool __b = (__a) ? true:false; templateAssert<__b>::there_has_been_a_compile_time_assertion();}
#else
#define TemplateAssert(a)
#endif
// --------------------------------------------------------------------------------------------------------------------------------
// Define a matrix that is N columns wide and M rows tall. This matrix is row-major.
//
// N can be thought of as the [N]umber of elements per vector, and M can be thought of as the number of vectors in the matrix
// --------------------------------------------------------------------------------------------------------------------------------

template <unsigned int N, unsigned int M, class T = float>
class	matrix
{
public:
				// Default constructor

				matrix() {}

				// Copy constructor

				matrix(const matrix &m)
				{
					memcpy(_data, m._data, N*M*sizeof(T));
				}

				// Initialize with three vectors

				matrix(const matrix<N, 1, T> &xv, const matrix<N, 1, T> &yv, const matrix<N, 1, T> &zv)
				{
					setXVector(xv);
					setYVector(yv);
					setZVector(zv);
				}

				// Initialize with four values (useful for vectors)

				matrix(const T &xv, const T &yv, const T &zv, const T &wv)
				{
					// This assertion guarantees that they are initializing the entire vector

					TemplateAssert(N == 4 && M == 1);

					x() = xv;
					y() = yv;
					z() = zv;
					w() = wv;
				}

				// Initialize with three values (useful for vectors)

				matrix(const T &xv, const T &yv, const T &zv)
				{
					// This assertion guarantees that they are initializing the entire vector

					TemplateAssert(N == 3 && M == 1);

					x() = xv;
					y() = yv;
					z() = zv;
				}

				// Initialize with two values (useful for vectors)

				matrix(const T &xv, const T &yv)
				{
					// This assertion guarantees that they are initializing the entire vector

					TemplateAssert(N == 2 && M == 1);

					x() = xv;
					y() = yv;
				}

				// These are handy 2D/3D/4D casts

				operator matrix<2, 1, T>()
				{
					TemplateAssert(M == 1);

					matrix<2, 1, T>	result;
					result.fill(0);

					int	c = N;
					if (c > 2) c = 2;

					for (unsigned int i = 0; i < c; i++) result(i,0) = (*this)(i,0);
					return result;
				}

				operator matrix<3, 1, T>()
				{
					TemplateAssert(M == 1);

					matrix<3, 1, T>	result;
					result.fill(0);

					int	c = N;
					if (c > 3) c = 3;

					for (unsigned int i = 0; i < c; i++) result(i,0) = (*this)(i,0);
					return result;
				}

				operator matrix<4, 1, T>()
				{
					TemplateAssert(M == 1);

					matrix<4, 1, T>	result;
					result.fill(0);
					result.w() = (T) 1;

					int	c = N;
					if (c > 4) c = 4;

					for (unsigned int i = 0; i < c; i++) result(i,0) = (*this)(i,0);
					return result;
				}

				// Return a zero'd matrix

static		matrix		zero()
				{
					matrix	result;
					result.fill((T) 0);
					return result;
				}

				// The infamous 'operator='

inline		matrix		&operator =(const matrix &m)
				{
					if (&m != this) memcpy(_data, m._data, N*M*sizeof(T));
					return *this;
				}
				//fill with array
inline		matrix		&operator =(const T *d)
				{
					for (int i=0; i<N; i++)
						for (int j=0; j<M; j++)
							(*this)(i,j) = d[i + j*M];
						return *this;
				}

				// Indexing: format = i down, j across

inline	const	T		&operator()(const unsigned int i, const unsigned int j) const
				{
					return _data[j*N+i];
				}

				// Indexing: format = i down, j across

inline		T		&operator()(const unsigned int i, const unsigned int j)
				{
					return _data[j*N+i];
				}

				// Matrix concatenation
				//
				// Specialized to follow the rules of matrix multiplication, for NxM * OxP:
				//   where M must be equal to O and resulting matrix is NxP. Otherwise, a
				//   compiler error will occur.
				//
				// Note that we use the >> operator. This is because of the lack of available
				//   operators, and also it serves as a reminder that the operations are from
				//   left-to-right (the convenient way.)

/*
#ifndef _MSC_VER
template <unsigned int P>
inline	const	matrix<N, P, T>	operator >>(const matrix<M, P, T> &m) const	{return concat(m);}
template <unsigned int P>
inline				operator >>=(const matrix<M, P, T> &m)		{*this = concat(m);}
template <unsigned int P>
inline	const	matrix<N, P, T>	concat(const matrix<M, P, T> &m) const
				{
					matrix<N, P, T>	result;
					result.fill((T) 0);

					for (unsigned int i = 0; i < N; i++)
					for (unsigned int j = 0; j < M; j++)
					for (unsigned int k = 0; k < P; k++)
					result(i,j) += (*this)(i,k) * m(k,j);

					return result;
				}
#else
				*/

// We're doing matrix*vector, which, as far as I can tell, doesn't work the same as vector*vector. Though, it would work the same
// if it was vector*matrix... so this sucker is specialized for that purpose

inline	const	matrix<N, 1, T>	operator >>(const matrix<M, 1, T> &m) const	{return concat(m);}
inline	void	operator >>=(const matrix<M, 1, T> &m)		{*this = concat(m);}
inline	const	matrix<N, 1, T>	concat(const matrix<M, 1, T> &m) const
				{
					matrix<N, 1, T>	result;
					result.fill((T) 0);

					for (unsigned int i = 0; i < N; i++)
					for (unsigned int j = 0; j < M; j++)
					result(i,0) += (*this)(j,i) * m(j,0);

					return result;
				}
//special homogenuos 3d vector *matrix(4,4) case
//for performance reasons, this is coded explicitly
template<class T> inline void	vec3_multiply_mat4(const matrix<4,4, T> &m, matrix<4,1, T> &res) const
				{
					//make sure is a vector!
					TemplateAssert(M==1 && N==4);
					//we're doing a transpose here to
					//make things work a little easier
					//ie multiply by row vec (vector3) instead of column

					res.x() =	m(0,0) * _data[0]
							+	m(1,0) * _data[1]
							+	m(2,0) * _data[2]
							+   m(3,0);	//*1

					res.y() =	m(0,1) * _data[0]
							+	m(1,1) * _data[1]
							+	m(2,1) * _data[2]
							+   m(3,1);

					res.z() =	m(0,2) * _data[0]
							+	m(1,2) * _data[1]
							+	m(2,2) * _data[2]
							+   m(3,2);
					res.w() = 1;
				}
inline	void	vec3_multiply_mat4(const T v[3], T resv[3]) const
				{
					//make sure this is a 4x4 matrix!
					TemplateAssert(M==4 && N==4);

					resv[0] =	(*this)(0,0) * v[0]
							+	(*this)(1,0) * v[1]
							+	(*this)(2,0) * v[2]
							+   (*this)(3,0);	//*1

					resv[1] =	(*this)(0,1) * v[0]
							+	(*this)(1,1) * v[1]
							+	(*this)(2,1) * v[2]
							+   (*this)(3,1);

					resv[2] =	(*this)(0,2) * v[0]
							+	(*this)(1,2) * v[1]
							+	(*this)(2,2) * v[2]
							+   (*this)(3,2);
				}
inline	void	vec3_multiply_mat4f(const float v[3], float resv[3]) const
				{
					//make sure this is a 4x4 matrix!
					TemplateAssert(M==4 && N==4);

					resv[0] =	(*this)(0,0) * v[0]
							+	(*this)(1,0) * v[1]
							+	(*this)(2,0) * v[2]
							+   (*this)(3,0);	//*1

					resv[1] =	(*this)(0,1) * v[0]
							+	(*this)(1,1) * v[1]
							+	(*this)(2,1) * v[2]
							+   (*this)(3,1);

					resv[2] =	(*this)(0,2) * v[0]
							+	(*this)(1,2) * v[1]
							+	(*this)(2,2) * v[2]
							+   (*this)(3,2);
				}
inline	void	mat3_multiply_vec3f(const float v[3], float resv[3]) const
				{
					//make sure this is a 3x3 matrix!
					TemplateAssert(M==3 && N==3);

					resv[0] =	(*this)(0,0) * v[0]
							+	(*this)(0,1) * v[1]
							+	(*this)(0,2) * v[2];

					resv[1] =	(*this)(1,0) * v[0]
							+	(*this)(1,1) * v[1]
							+	(*this)(1,2) * v[2];

					resv[2] =	(*this)(2,0) * v[0]
							+	(*this)(2,1) * v[1]
							+	(*this)(2,2) * v[2];
				}
inline	void	mat3_multiply_vec3(const T v[3], T resv[3]) const
				{
					//make sure this is a 3x3 matrix!
					TemplateAssert(M==3 && N==3);

					resv[0] =	(*this)(0,0) * v[0]
							+	(*this)(0,1) * v[1]
							+	(*this)(0,2) * v[2];

					resv[1] =	(*this)(1,0) * v[0]
							+	(*this)(1,1) * v[1]
							+	(*this)(1,2) * v[2];

					resv[2] =	(*this)(2,0) * v[0]
							+	(*this)(2,1) * v[1]
							+	(*this)(2,2) * v[2];
				}
inline	void	mat3_multiply_vec3(T v[3]) const
				{
					T r[3];
					mat3_multiply_vec3(v, r);
					v[0] = r[0]; v[1] = r[1]; v[2] = r[2];
				}
inline	void	mat3_multiply_vec3f(float v[3]) const
				{
					float r[3];
					mat3_multiply_vec3f(v, r);
					v[0] = r[0]; v[1] = r[1]; v[2] = r[2];
				}
inline	void	vec3_multiply_mat4f(float v[3]) const
				{
					float w[3];
					//make sure this is a 4x4 matrix!
					TemplateAssert(M==4 && N==4);

					w[0] =		(*this)(0,0) * v[0]
							+	(*this)(1,0) * v[1]
							+	(*this)(2,0) * v[2]
							+   (*this)(3,0);	//*1

					w[1] =		(*this)(0,1) * v[0]
							+	(*this)(1,1) * v[1]
							+	(*this)(2,1) * v[2]
							+   (*this)(3,1);

					w[2] =		(*this)(0,2) * v[0]
							+	(*this)(1,2) * v[1]
							+	(*this)(2,2) * v[2]
							+   (*this)(3,2);
					v[0] = w[0]; v[1] = w[1]; v[2] = w[2];
				}
inline	void	vec3_multiply_mat4(T v[3]) const
				{
					T r[3];
					vec3_multiply_mat4(v, r);
					v[0] = r[0]; v[1] = r[1]; v[2] = r[2];
				}
inline	const	matrix<N, 2, T>	operator >>(const matrix<M, 2, T> &m) const	{return concat(m);}
inline	void	operator >>=(const matrix<M, 2, T> &m)		{*this = concat(m);}
inline	const	matrix<N, 2, T>	concat(const matrix<M, 2, T> &m) const
				{
					matrix<N, 2, T>	result;
					result.fill((T) 0);

					for (unsigned int i = 0; i < N; i++)
					for (unsigned int j = 0; j < M; j++)
					for (unsigned int k = 0; k < 2; k++)
					result(i,j) += (*this)(i,k) * m(k,j);

					return result;
				}

inline	const	matrix<N, 3, T>	operator >>(const matrix<M, 3, T> &m) const	{return concat(m);}
inline	void	operator >>=(const matrix<M, 3, T> &m)		{*this = concat(m);}
inline	const	matrix<N, 3, T>	concat(const matrix<M, 3, T> &m) const
				{
					matrix<N, 3, T>	result;
					result.fill((T) 0);

					for (unsigned int i = 0; i < N; i++)
					for (unsigned int j = 0; j < M; j++)
					for (unsigned int k = 0; k < 3; k++)
					result(i,j) += (*this)(i,k) * m(k,j);

					return result;
				}

inline	const	matrix<N, 4, T>	operator >>(const matrix<M, 4, T> &m) const	{return concat(m);}
inline	void	operator >>=(const matrix<M, 4, T> &m)		{*this = concat(m);}
inline	const	matrix<N, 4, T>	concat(const matrix<M, 4, T> &m) const
				{
					matrix<N, 4, T>	result;
					result.fill((T) 0);

					for (unsigned int i = 0; i < N; i++)
					for (unsigned int j = 0; j < M; j++)
					for (unsigned int k = 0; k < 4; k++)
					result(i,j) += (*this)(i,k) * m(k,j);

					return result;
				}
//#endif
				// 3D Vector cross product
				//
				// Note that the cross product is specifically a 3-dimensional operation. So this routine
				// will fill the remaining elements (if any) with the contents of this->_data[...]
				//
				// Specialized for Nx1 matrices (i.e. vectors)

inline	void	cross(const matrix<N, 1, T> &m)
				{
					TemplateAssert(N >= 3);

					T	tx = y() * m.z() - z() * m.y();
					T	ty = z() * m.x() - x() * m.z();
					T	tz = x() * m.y() - y() * m.x();
					x() = tx;
					y() = ty;
					z() = tz;
				}

inline	const	matrix<N, 1, T>	operator %(const matrix<N, 1, T> &m) const
				{
					matrix<N, 1, T>	result = *this;
					result.cross(m);
					return result;
				}

inline	const	matrix	operator %=(const matrix &m)
				{
					cross(m);
					return *this;
				}

				// Vector dot product
				//
				// Specialized for Nx1 matrices (i.e. vectors)
inline	const	T	dot(const matrix<N, 1, T> &m) const
				{
					T	result = (T) 0;
					for (unsigned int i = 0; i < N; i++) result += _data[i] * m.data()[i];
					return result;
				}

inline	const	T	operator ^(const matrix<N, 1, T> &m) const
				{
					return dot(m);
				}

				// Component-wise multiplication with matrix
inline	const	matrix	operator *(const matrix &m) const
				{
					matrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = _data[i] * m._data[i];
					return result;
				}

				// Component-wise multiplication with scalar
inline	const	matrix	operator *(const T &value) const
				{
					matrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = _data[i] * value;
					return result;
				}

				// Component-wise multiplication with matrix (into self)
inline	const	matrix	operator *=(const matrix &m)
				{
					for (unsigned int i = 0; i < N*M; i++) _data[i] *= m._data[i];
					return *this;
				}

				// Component-wise multiplication with scalar (into self)
inline	const	matrix	operator *=(const T &value)
				{
					for (unsigned int i = 0; i < N*M; i++) _data[i] *= value;
					return *this;
				}

				// Component-wise division with matrix
inline	const	matrix	operator /(const matrix &m) const
				{
					matrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = _data[i] / m._data[i];
					return result;
				}

				// Component-wise division with scalar
inline	const	matrix	operator /(const T &value) const
				{
					matrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = _data[i] / value;
					return result;
				}

				// Component-wise division with scalar (scalar / component)
inline	const	matrix	inverseDivide(const T &value) const
				{
					matrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = value / _data[i];
					return result;
				}

				// Component-wise division with matrix (into self)
inline	const	matrix	operator /=(const matrix &m)
				{
					for (unsigned int i = 0; i < N*M; i++) _data[i] /= m._data[i];
					return *this;
				}

				// Component-wise division with scalar (into self)
inline	const	matrix	operator /=(const T &value)
				{
					for (unsigned int i = 0; i < N*M; i++) _data[i] /= value;
					return *this;
				}

				// Component-wise addition with matrix
inline	const	matrix	operator +(const matrix &m) const
				{
					matrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = _data[i] + m._data[i];
					return result;
				}

				// Component-wise addition with scalar
inline	const	matrix	operator +(const T &value) const
				{
					matrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = _data[i] + value;
					return result;
				}

				// Component-wise addition with matrix (into self)
inline	const	matrix	operator +=(const matrix &m)
				{
					for (unsigned int i = 0; i < N*M; i++) _data[i] += m._data[i];
					return *this;
				}

				// Component-wise addition with scalar (into self)
inline	const	matrix	operator +=(const T &value)
				{
					for (unsigned int i = 0; i < N*M; i++) _data[i] += value;
					return *this;
				}

				// Component-wise negation
inline	const	matrix	operator -() const
				{
					matrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = -_data[i];
					return result;
				}

				// Component-wise subtraction with matrix
inline	const	matrix	operator -(const matrix &m) const
				{
					matrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = _data[i] - m._data[i];
					return result;
				}

				// Component-wise subtraction with scalar
inline	const	matrix	operator -(const T &value) const
				{
					matrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = _data[i] - value;
					return result;
				}

				// Component-wise subtraction with scalar (scalar - component)
inline	const	matrix	inverseSubtract(const T &value) const
				{
					matrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = value - _data[i];
					return result;
				}

				// Component-wise subtraction with matrix (into self)
inline	const	matrix	operator -=(const matrix &m)
				{
					for (unsigned int i = 0; i < N*M; i++) _data[i] -= m._data[i];
					return *this;
				}

				// Component-wise subtraction with scalar (into self)
inline	const	matrix	operator -=(const T &value)
				{
					for (unsigned int i = 0; i < N*M; i++) _data[i] -= value;
					return *this;
				}

				// Total all components
inline		T		total()
				{
					T	tot = (T) 0;
					for (unsigned int i = 0; i < N*M; i++) tot += _data[i];
					return tot;
				}

				// Comparison for equality
inline	const	bool		operator ==(const matrix &m) const
				{
					for (unsigned int i = 0; i < N*M; i++) if (_data[i] != m._data[i]) return false;
					return true;
				}

				// Comparison for inequality
inline	const	bool		operator !=(const matrix &m) const
				{
					return !(*this == m);
				}

inline	const	bool		operator <(const matrix &m) const
				{
					for (unsigned int i = 0; i < N*M; i++) if (_data[i] >= m._data[i]) return false;
					return true;
				}

inline	const	bool		operator <=(const matrix &m) const
				{
					for (unsigned int i = 0; i < N*M; i++) if (_data[i] > m._data[i]) return false;
					return true;
				}

inline	const	bool		operator >(const matrix &m) const
				{
					for (unsigned int i = 0; i < N*M; i++) if (_data[i] <= m._data[i]) return false;
					return true;
				}

inline	const	bool		operator >=(const matrix &m) const
				{
					for (unsigned int i = 0; i < N*M; i++) if (_data[i] < m._data[i]) return false;
					return true;
				}
inline	const matrix<N, 1, T> combine(T this_scale, const matrix<N, 1, T> &m, T scale)
				{
					matrix<N,1,T> res;
					for (int i=0; i<N; i++)
					{
						res._data[i] = (this_scale * _data[i]) + (m._data[i] * scale);
					}
					return res;
				}
				// Orthogonal transpose
				//
				// Note that matrix must be square (i.e. N == M)

inline		void	transpose()
				{
					TemplateAssert(N == M);

					// Transpose the matrix

					matrix result;
					for (unsigned int j = 0; j < M; j++)
					{
						for (unsigned int i = 0; i < N; i++)
						{
							result(j,i) = (*this)(i,j);
						}
					}
					*this = result;
				}

				// Returns determinant of the matrix (4x4 only)
inline		T	determinant()
				{
					TemplateAssert(N == 4 && M == 4);

					matrix	&m = *this;
					return	  (m(0,0) * m(1,1) - m(1,0) * m(0,1)) * (m(2,2) * m(3,3) - m(3,2) * m(2,3))
						- (m(0,0) * m(2,1) - m(2,0) * m(0,1)) * (m(1,2) * m(3,3) - m(3,2) * m(1,3))
						+ (m(0,0) * m(3,1) - m(3,0) * m(0,1)) * (m(1,2) * m(2,3) - m(2,2) * m(1,3))
						+ (m(1,0) * m(2,1) - m(2,0) * m(1,1)) * (m(0,2) * m(3,3) - m(3,2) * m(0,3))
						- (m(1,0) * m(3,1) - m(3,0) * m(1,1)) * (m(0,2) * m(2,3) - m(2,2) * m(0,3))
						+ (m(2,0) * m(3,1) - m(3,0) * m(2,1)) * (m(0,2) * m(1,3) - m(1,2) * m(0,3));
				}

				// Inverts the matrix (4x4 only)

inline		void		invert()
				{
					TemplateAssert(N == 4 && M == 4);

					T	d = determinant();
					if (d == 0.0) return;

					d = 1.0 / d;

					matrix	&m = *this;
					matrix	result;
					result(0,0) = d * (m(1,1) * (m(2,2) * m(3,3) - m(3,2) * m(2,3)) + m(2,1) * (m(3,2) * m(1,3) - m(1,2) * m(3,3)) + m(3,1) * (m(1,2) * m(2,3) - m(2,2) * m(1,3)));
					result(1,0) = d * (m(1,2) * (m(2,0) * m(3,3) - m(3,0) * m(2,3)) + m(2,2) * (m(3,0) * m(1,3) - m(1,0) * m(3,3)) + m(3,2) * (m(1,0) * m(2,3) - m(2,0) * m(1,3)));
					result(2,0) = d * (m(1,3) * (m(2,0) * m(3,1) - m(3,0) * m(2,1)) + m(2,3) * (m(3,0) * m(1,1) - m(1,0) * m(3,1)) + m(3,3) * (m(1,0) * m(2,1) - m(2,0) * m(1,1)));
					result(3,0) = d * (m(1,0) * (m(3,1) * m(2,2) - m(2,1) * m(3,2)) + m(2,0) * (m(1,1) * m(3,2) - m(3,1) * m(1,2)) + m(3,0) * (m(2,1) * m(1,2) - m(1,1) * m(2,2)));
					result(0,1) = d * (m(2,1) * (m(0,2) * m(3,3) - m(3,2) * m(0,3)) + m(3,1) * (m(2,2) * m(0,3) - m(0,2) * m(2,3)) + m(0,1) * (m(3,2) * m(2,3) - m(2,2) * m(3,3)));
					result(1,1) = d * (m(2,2) * (m(0,0) * m(3,3) - m(3,0) * m(0,3)) + m(3,2) * (m(2,0) * m(0,3) - m(0,0) * m(2,3)) + m(0,2) * (m(3,0) * m(2,3) - m(2,0) * m(3,3)));
					result(2,1) = d * (m(2,3) * (m(0,0) * m(3,1) - m(3,0) * m(0,1)) + m(3,3) * (m(2,0) * m(0,1) - m(0,0) * m(2,1)) + m(0,3) * (m(3,0) * m(2,1) - m(2,0) * m(3,1)));
					result(3,1) = d * (m(2,0) * (m(3,1) * m(0,2) - m(0,1) * m(3,2)) + m(3,0) * (m(0,1) * m(2,2) - m(2,1) * m(0,2)) + m(0,0) * (m(2,1) * m(3,2) - m(3,1) * m(2,2)));
					result(0,2) = d * (m(3,1) * (m(0,2) * m(1,3) - m(1,2) * m(0,3)) + m(0,1) * (m(1,2) * m(3,3) - m(3,2) * m(1,3)) + m(1,1) * (m(3,2) * m(0,3) - m(0,2) * m(3,3)));
					result(1,2) = d * (m(3,2) * (m(0,0) * m(1,3) - m(1,0) * m(0,3)) + m(0,2) * (m(1,0) * m(3,3) - m(3,0) * m(1,3)) + m(1,2) * (m(3,0) * m(0,3) - m(0,0) * m(3,3)));
					result(2,2) = d * (m(3,3) * (m(0,0) * m(1,1) - m(1,0) * m(0,1)) + m(0,3) * (m(1,0) * m(3,1) - m(3,0) * m(1,1)) + m(1,3) * (m(3,0) * m(0,1) - m(0,0) * m(3,1)));
					result(3,2) = d * (m(3,0) * (m(1,1) * m(0,2) - m(0,1) * m(1,2)) + m(0,0) * (m(3,1) * m(1,2) - m(1,1) * m(3,2)) + m(1,0) * (m(0,1) * m(3,2) - m(3,1) * m(0,2)));
					result(0,3) = d * (m(0,1) * (m(2,2) * m(1,3) - m(1,2) * m(2,3)) + m(1,1) * (m(0,2) * m(2,3) - m(2,2) * m(0,3)) + m(2,1) * (m(1,2) * m(0,3) - m(0,2) * m(1,3)));
					result(1,3) = d * (m(0,2) * (m(2,0) * m(1,3) - m(1,0) * m(2,3)) + m(1,2) * (m(0,0) * m(2,3) - m(2,0) * m(0,3)) + m(2,2) * (m(1,0) * m(0,3) - m(0,0) * m(1,3)));
					result(2,3) = d * (m(0,3) * (m(2,0) * m(1,1) - m(1,0) * m(2,1)) + m(1,3) * (m(0,0) * m(2,1) - m(2,0) * m(0,1)) + m(2,3) * (m(1,0) * m(0,1) - m(0,0) * m(1,1)));
					result(3,3) = d * (m(0,0) * (m(1,1) * m(2,2) - m(2,1) * m(1,2)) + m(1,0) * (m(2,1) * m(0,2) - m(0,1) * m(2,2)) + m(2,0) * (m(0,1) * m(1,2) - m(1,1) * m(0,2)));
					*this = result;
				}

				// Fill the matrix with a single value

inline		void		fill(const T &value)
				{
					T	*ptr = _data;
					for (unsigned int i = 0; i < N*M; i++, ptr++) *ptr = value;
				}

				// Generate identity matrix
				//
				// Note that matrix must be square (i.e. N == M)

static	const	matrix<N, M, T>	identity()
				{
					TemplateAssert(N == M);

					// Make it identity

					matrix<N, M, T>	result;
					T		*ptr = result._data;
					for (unsigned int j = 0; j < M; j++)
					{
						for (unsigned int i = 0; i < N; i++, ptr++)
						{
							if (i == j)	*ptr = (T) 1;
							else		*ptr = (T) 0;
						}
					}
					return result;
				}

				// Generate rotation matrix (3x3) for rotation about the X axis (i.e. rotation happens along the Y/Z plane)

static	const	matrix<N, M, T>	xRotation(const T &theta)
				{
					TemplateAssert(N >= 3 && M >= 3);

					// Start with identity

					matrix<N, M, T>	result = identity();

					// Fill it in

					T	ct = (T) cos((double) theta);
					T	st = (T) sin((double) theta);
					result(1,1) =  ct;
					result(2,1) =  st;
					result(1,2) = -st;
					result(2,2) =  ct;
					return result;
				}

				// Generate rotation matrix (3x3) for rotation about the Y axis (i.e. rotation happens along the X/Z plane)
				//
				// Note that the matrix must be a minimum of 3x3

static	const	matrix<N, M, T>	yRotation(const T &theta)
				{
					TemplateAssert(N >= 3 && M >= 3);

					// Start with identity

					matrix<N, M, T>	result = identity();

					// Fill it in

					T	ct = (T) cos((double) theta);
					T	st = (T) sin((double) theta);
					result(0,0) =  ct;
					result(2,0) =  st;
					result(0,2) = -st;
					result(2,2) =  ct;
					return result;
				}

				// Generate rotation matrix (3x3) for rotation about the Z axis (i.e. rotation happens along the X/Y plane)
				//
				// Note that this matrix is allowed to be only 2x2 as this is a 2-D rotation

static	const	matrix<N, M, T>	zRotation(const T &theta)
				{
					TemplateAssert(N >= 3 && M >= 3);

					// Start with identity

					matrix<N, M, T>	result = identity();

					// Fill it in

					T	ct = (T) cos((double) theta);
					T	st = (T) sin((double) theta);
					result(0,0) =  ct;
					result(1,0) = -st;
					result(0,1) =  st;
					result(1,1) =  ct;
					return result;
				}

				// Generate a concatenated rotation matrix (3x3) for rotation about all axes (i.e. arbitrary rotation)

static	const	matrix		rotation(const T &xTheta, const T &yTheta, const T &zTheta)
				{
					return zRotation(zTheta) >> yRotation(yTheta) >> xRotation(xTheta);
				}

static const matrix gluLookAt( const T *eye, const T *target, const T *up)
{
    float forward[3], side[3], up[3];
    GLfloat m[4][4];

    forward[0] = centerx - eyex;
    forward[1] = centery - eyey;
    forward[2] = centerz - eyez;

    up[0] = upx;
    up[1] = upy;
    up[2] = upz;

    normalize(forward);

    /* Side = forward x up */
    cross(forward, up, side);
    normalize(side);

    /* Recompute up as: up = side x forward */
    cross(side, forward, up);

    __gluMakeIdentityf(&m[0][0]);
    m[0][0] = side[0];
    m[1][0] = side[1];
    m[2][0] = side[2];

    m[0][1] = up[0];
    m[1][1] = up[1];
    m[2][1] = up[2];

    m[0][2] = -forward[0];
    m[1][2] = -forward[1];
    m[2][2] = -forward[2];

    glMultMatrixf(&m[0][0]);
    glTranslated(-eyex, -eyey, -eyez);
}
				// Generate a 'look-at' matrix. Must be a 3x3 result because this routine uses a cross product

static	const	matrix<3, 3, T>	lookat(const matrix<3, 1, T> &v, const T &theta = (T) 0)
				{
					matrix<3, 1, T>	zAxis = v;
					zAxis.normalize();

					matrix<3, 1, T>	yAxis;
					yAxis.fill((T) 0);

					// Handle the degenerate case... (this acts exactly like 3DS-R4)

					if (!zAxis.x() && !zAxis.z())	yAxis.z() = -zAxis.y();
					else yAxis.y() = (T) 1;

					matrix<3, 1, T>	xAxis = yAxis % zAxis;
					xAxis.normalize();

					yAxis = xAxis % zAxis;
					yAxis.normalize();
					yAxis = -yAxis;

					matrix<3, 3, T>	m(xAxis, yAxis, zAxis);
					return m >> zRotation(theta);
				}

				// Generate a scale matrix

static		matrix		scale(const matrix<N, 1, T> &m)
				{
					TemplateAssert(N <= M);

					matrix	result;
					result = identity();
					for (unsigned int i = 0; i < N; i++)
					{
						result(i,i) *= m(i,0);
					}
					return result;
				}
static		matrix 		scale(const T v[N])
				{
					matrix	result;
					result = identity();
					for (unsigned int i = 0; i < N; i++)
					{
						result(i,i) *= v[i];
					}
					return result;
				}
				// Generate a translation matrix
static		matrix		translation(const T *v)
				{
					matrix	result;
					result = identity();
					for (unsigned int i = 0; i < M-1; i++)
					{
						result(N-1,i) += v[i];
					}
					return result;
				}
static		matrix		translation(const matrix<N, 1, T> &m)
				{
					TemplateAssert(M <= N);

					matrix	result;
					result = identity();
					for (unsigned int i = 0; i < M-1; i++)
					{
						result(N-1,i) += m(i,0);
					}
					return result;
				}
				//translate this
void		translate(const T v[N])
			{
				for (unsigned int i = 0; i < M-1; i++)
				{
					(*this)(N-1,i) += v[i];
				}
			}
			//inv translate this
void		inv_translate(const T v[N])
			{
				for (unsigned int i = 0; i < M-1; i++)
				{
					(*this)(N-1,i) -= v[i];
				}
			}
				// Generate a shear matrix
static		matrix		shear(const T x, const T y)
				{
					TemplateAssert(N > 1 && M > 1);

					matrix	result;
					result = identity();
					result(1,0) = x;
					result(0,1) = y;
					return result;
				}

				// Generate a (4x4) perspective projection matrix (as per D3D)

static	const	matrix<4, 4, T>	projectPerspectiveD3D(const T &fov, const T &aspect, const T &n, const T &f)
				{
					T	w  = 1 / tan(fov / (T) 2);
					T	h  = 1 / tan(fov / (T) 2);
					if (aspect > 1.0)	w /= aspect;
					else			h *= aspect;
					T	q  = f / (f - n);

					matrix<4, 4, T>	result;
					result.fill((T)0);
					result(0,0) = w;
					result(1,1) = h;
					result(2,2) = q;
					result(3,2) = -q*n;
					result(2,3) = 1;
					return result;
				}
				// Generate a (4x4) perspective projection matrix (as per glu)
static	const	matrix<4, 4, T>	projectPerspectiveGl(const T &fov_rad, const T &aspect, const T &n, const T &f)
				{
					matrix<4, 4, T>	m = matrix<4, 4, T>::identity();
					T sine, cotangent, deltaZ;
					double fov = fov_rad * 0.5;
				    deltaZ = f - n;
					sine = sin(fov);
					if ((deltaZ == 0) || (sine == 0) || (aspect == 0)) {
						return m;
					}
					cotangent = cos(fov) / sine;

					m(0,0) = cotangent / aspect;
					m(1,1) = cotangent;
					m(2,2) = -(f + n) / deltaZ;
					m(2,3) = -1;
					m(3,2) = -2 * n * f / deltaZ;
					m(3,3) = 0;

					return m;
				}
				// Generate a (4x4) perspective projection matrix (as per Blinn)

static	const	matrix<4, 4, T>	projectPerspectiveBlinn(const T &fov, const T &aspect, const T &n, const T &f)
				{
					T	w  = cos(fov / (T) 2);
					T	h  = cos(fov / (T) 2);
					if (aspect > 1.0)	w /= aspect;
					else			h *= aspect;
					T	s  = sin(fov / (T) 2); // ???
					T	d  = (T) 1 - n/f;

					matrix<4, 4, T>	result;
					result.fill((T)0);
					result(0,0) = w;
					result(1,1) = h;
					result(2,2) = s / d;
					result(3,2) = -(s * n / d);
					result(2,3) = s;
					return result;
				}

				// Generate a (4x4) perspective projection matrix (as per glFrustum)

static	const	matrix<4, 4, T>	projectPerspectiveGlFrustum(const T &l, const T &r, const T &b, const T &t, const T &n, const T &f)
				{
					matrix<4, 4, T>	result;
					result.fill((T)0);
					result(0,0) = (2*n)/(r-l);
					result(2,0) = (r+l)/(r-l);
					result(1,1) = (2*n)/(t-b);
					result(2,1) = (t+b)/(t-b);
					result(2,2) = (-(f+n))/(f-n);
					result(3,2) = (-2*f*n)/(f-n);
					result(2,3) = -1;
					return result;
				}

				// Generate a (4x4) orthogonal projection matrix (as per glOrtho)

static	const	matrix<4, 4, T>	projectGlOrtho(const T &l, const T &r, const T &b, const T &t, const T &n, const T &f)
				{
					matrix<4, 4, T>	result;
					result.fill((T)0);
					result(0,0) = 2/(r-l);
					result(3,0) = -((r+l)/(r-l));
					result(1,1) = 2/(t-b);
					result(3,1) = -((t+b)/(t-b));
					result(2,2) = (-2)/(f-n);
					result(3,2) = -((f+n)/(f-n));
					result(3,3) = 1;
					return result;
				}

				// Generate a (4x4) orthogonal projection matrix (standard -- maps to z=0 plane)

static	const	matrix<4, 4, T>	projectOrtho(const T &xScale, const T &yScale)
				{
					matrix<4, 4, T>	result;
					result.fill((T)0);
					result(0,0) = xScale;
					result(1,1) = yScale;
					result(3,3) = 1;
					return result;
				}

				// Extract the X vector from the matrix
				//
				// Note that the matrix must be a minimum of 3x3
inline		void extractXVector(T *v) const
				{
					TemplateAssert(M > 0);

					for (unsigned int i = 0; i < N; i++)
						v[i] = (*this)(i,0);
				}
inline		matrix<N, 1, T>	extractXVector() const
				{
					TemplateAssert(M > 0);

					matrix<N, 1, T>	result;
					for (unsigned int i = 0; i < N; i++)
						result(i,0) = (*this)(i,0);
					return result;
				}

				// Extract the Y vector from the matrix
				//
				// Note that the matrix must be a minimum of 3x3
inline		void extractYVector(T *v) const
				{
					TemplateAssert(M > 1);

					for (unsigned int i = 0; i < N; i++)
						v[i] = (*this)(i,1);
				}
inline		matrix<N, 1, T>	extractYVector() const
				{
					TemplateAssert(M > 1);

					matrix<N, 1, T>	result;
					for (unsigned int i = 0; i < N; i++)
						result(i,0) = (*this)(i,1);
					return result;
				}

				// Extract the Z vector from the matrix
				//
				// Note that the matrix must be a minimum of 3x3
inline		void extractZVector(T *v) const
				{
					TemplateAssert(M > 2);

					for (unsigned int i = 0; i < N; i++)
						v[i] = (*this)(i,2);
				}
inline		matrix<N, 1, T>	extractZVector() const
				{
					TemplateAssert(M > 2);

					matrix<N, 1, T>	result;
					for (unsigned int i = 0; i < N; i++)
						result(i,0) = (*this)(i,2);
					return result;
				}
inline		void extractVector(int idx, T *v) const
				{
					for (unsigned int i = 0; i < N; i++)
						v[i] = (*this)(i,idx);
				}
				// Replace the X vector within matrix
				//
				// Note that the matrix must be a minimum of 3x3

inline		void		setXVector(const matrix<N, 1, T> &m)
				{
					for (unsigned int i = 0; i < N; i++)
						(*this)(i,0) = m(i,0);
				}
inline		void		setXVector(const T *m)
				{
					for (unsigned int i = 0; i < N; i++)
						(*this)(i,0) = m[i];
				}
				// Replace the Y vector within matrix
				//
				// Note that the matrix must be a minimum of 3x3

inline		void		setYVector(const matrix<N, 1, T> &m)
				{
					for (unsigned int i = 0; i < N; i++)
						(*this)(i,1) = m(i,0);
				}
inline		void		setYVector(const T *m)
				{
					for (unsigned int i = 0; i < N; i++)
						(*this)(i,1) = m[i];
				}
				// Replace the Z vector within matrix
				//
				// Note that the matrix must be a minimum of 3x3

inline		void		setZVector(const matrix<N, 1, T> &m)
				{
					for (unsigned int i = 0; i < N; i++)
						(*this)(i,2) = m(i,0);
				}
inline		void		setZVector(const T *m)
				{
					for (unsigned int i = 0; i < N; i++)
						(*this)(i,2) = m[i];
				}
inline		void		setVector(int idx, const T *m)
				{
					for (unsigned int i = 0; i < N; i++)
						(*this)(i,idx) = m[i];
				}
				// Vector length calculation (squared)
				//
				// Specialized for Nx1 matrices (i.e. vectors)

inline	const	T		lengthSquared() const
				{
					TemplateAssert(M == 1);

					return dot(*this);
				}

				// Vector length calculation
				//
				// Specialized for Nx1 matrices (i.e. vectors)

inline	const	T		length() const
				{
					TemplateAssert(M == 1);

					return sqrt(lengthSquared());
				}

				// Vector length
				//
				// Specialized for Nx1 matrices (i.e. vectors)

inline		void		setLength(const T &len)
				{
					TemplateAssert(M == 1);

					T	l = len / length();

					for (int i=0; i<N; i++)
						(*this)(i,0) *= l;
				}

				// Vector length
				//
				// Specialized for Nx1 matrices (i.e. vectors)

inline		T		distance(const matrix &m) const
				{
					TemplateAssert(M == 1);

					matrix	temp = *this - m;
					return temp.length();
				}

				// Normalize
				//
				// Specialized for Nx1 matrices (i.e. vectors)

inline		void		normalize()
				{
					setLength((T) 1);
				}

				// Normalize an orthogonal matrix (i.e. make sure the internal vectors are all perpendicular)
				//
				// Note that the matrix must be a 3x3, due to the fact that this routine uses a cross product

inline		void		orthoNormalize()
				{
					TemplateAssert(N == 3 && M == 3);

					matrix<N, 1, T>	xVector = extractXVector();
					matrix<N, 1, T>	yVector = extractYVector();
					matrix<N, 1, T>	zVector = extractZVector();

					xVector -= yVector * (xVector * yVector);
					xVector -= zVector * (xVector * zVector);
					xVector.normalize();

					yVector -= xVector * (yVector * xVector);
					yVector -= zVector * (yVector * zVector);
					yVector.normalize();

					zVector = xVector % yVector;

					setXVector(xVector);
					setYVector(yVector);
					setZVector(zVector);
				}

				// Absolute value of all components

inline		void		abs()
				{
					for (unsigned int i = 0; i < N*M; i++)
						_data[i] = (T) fabs((double) _data[i]);
				}

				// Specialized 'convenience accessors' for vectors, points, etc.
				//
				// Note that the matrix must have a value of N large enough to store the value
				// in question and M must always be 1.

inline	const	T 		&x() const {TemplateAssert(N > 0 && M == 1); return _data[0];}
inline		T		&x()       {TemplateAssert(N > 0 && M == 1); return _data[0];}
inline	const	T 		&y() const {TemplateAssert(N > 1 && M == 1); return _data[1];}
inline		T		&y()       {TemplateAssert(N > 1 && M == 1); return _data[1];}
inline	const	T 		&z() const {TemplateAssert(N > 2 && M == 1); return _data[2];}
inline		T		&z()       {TemplateAssert(N > 2 && M == 1); return _data[2];}
inline	const	T 		&w() const {TemplateAssert(N > 3 && M == 1); return _data[3];}
inline		T		&w()       {TemplateAssert(N > 3 && M == 1); return _data[3];}

				// Only use this if you need to... prefer operator(int,int) for access

inline	const	T		*data() const {return _data;}
inline		T		*data()       {return _data;}

				// Dimensions

inline		unsigned int	width() const {return N;}
inline		unsigned int	height() const {return M;}

				//gl - thses are all 4x4 matrix operations
				//added by Faraz Ravi igloosoft 2002
inline		void	loadGLmodel()
				{
					TemplateAssert(N == 4 && M == 4);

					GLdouble mat[16];
					glGetDoublev(GL_MODELVIEW_MATRIX, mat);

					for (unsigned int i = 0; i < N; i++)
					{
						for (unsigned int j = 0; j < M; j++)
						{
							(*this)(i,j) = (T)mat[i * 4 + j];
						}
					}
				}
inline		void	getGLmatrix16(T *m) const
				{
					TemplateAssert(N == 4 && M == 4);

					int c = 0;
					for (unsigned int i = 0; i < N; i++)
						for (unsigned int j = 0; j < M; j++)
							m[c++] = (*this)(i,j);
				}
			/* unmatrix - Decompose a non-degenerate 4x4 transformation matrix into
			 * 	the sequence of transformations that produced it.
			 * [Sx][Sy][Sz][Shearx/y][Sx/z][Sz/y][Rx][Ry][Rz][Tx][Ty][Tz][P(x,y,z,w)]
			 *
			 * The coefficient of each transformation is returned in the corresponding
			 * element of the vector tran.
			 *
			 * Returns 1 upon success, 0 if the matrix is singular.
			 */
#define U_PERSPX	12
#define U_PERSPY	13
#define U_PERSPZ	14
#define U_PERSPW	15
#define U_TRANSX	9
#define U_TRANSY	10
#define U_TRANSZ	11
#define U_SCALEX	0
#define U_SCALEY	1
#define U_SCALEZ	2
#define U_SHEARXY	3
#define U_SHEARYZ	4
#define U_SHEARXZ	5
#define U_ROTATEX	6
#define U_ROTATEY	7
#define U_ROTATEZ	8

			int		untransform(double tran[16]) const
				{
					TemplateAssert(N == 4 && M == 4);

 					register int i, j;
 					matrix<4,4,T> locmat;
 					matrix<4,4,T> pmat, invpmat, tinvpmat;

 					/* Vector4 type and functions need to be added to the common set. */
 					matrix<4,1,T> prhs, psol;
 					matrix<3,1,double> row[3], pdum3;

 					locmat = *this;

 					/* Normalize the matrix. */
 					if (locmat(3,3) == 0 )
 						return 0;
 					for (i=0; i<4;i++)
 						for ( j=0; j<4; j++ )
 							locmat(i,j) /= locmat(3,3);

					/* pmat is used to solve for perspective, but it also provides
					 * an easy way to test for singularity of the upper 3x3 component.
					 */
					pmat = locmat;
 					for ( i=0; i<3; i++ )
 						pmat(i,3) = 0;
 					pmat(3,3) = 1;

 					if (pmat.determinant() == 0.0)
 						return 0;

					/* First, isolate perspective.  This is the messiest. */
 					if ( locmat(0,3) != 0 || locmat(1,3) != 0 || locmat(2,3) != 0 )
					{
 						/* prhs is the right hand side of the equation. */
 						prhs(0,0) = locmat(0,3);
 						prhs(1,0) = locmat(1,3);
 						prhs(2,0) = locmat(2,3);
 						prhs(3,0) = locmat(3,3);

 						/* Solve the equation by inverting pmat and multiplying
 						 * prhs by the inverse.  (This is the easiest way, not
 						 * necessarily the best.)*/
 						invpmat = pmat;
						invpmat.invert();

						tinvpmat = invpmat;
						tinvpmat.transpose();

						psol = tinvpmat >> prhs;//V4MulPointByMatrix(&prhs, &tinvpmat, &psol);

						tran[U_PERSPX] = psol.x();
 						tran[U_PERSPY] = psol.y();
 						tran[U_PERSPZ] = psol.z();
 						tran[U_PERSPW] = psol.w();

						/* Clear the perspective partition. */
 						locmat(0,3) = locmat(1,3) = locmat(2,3) = 0;
 						locmat(3,3) = 1;
 					}
					else		/* No perspective. */
 						tran[U_PERSPX] = tran[U_PERSPY] = tran[U_PERSPZ] = tran[U_PERSPW] = 0;

				/* Next take care of translation (easy). */
 				for ( i=0; i<3; i++ ) {
 					tran[U_TRANSX + i] = locmat(3,i);
 					locmat(3,i) = 0;
 					}

 				/* Now get scale and shear. */
 				for ( i=0; i<3; i++ ) {
 					row[i](0,0) = locmat(i,0);
 					row[i](1,0) = locmat(i,1);
 					row[i](2,0) = locmat(i,2);
 				}

 				/* Compute X scale factor and normalize first row. */
 				tran[U_SCALEX] = row[0].length();
 				row[0].normalize();

 				/* Compute XY shear factor and make 2nd row orthogonal to 1st. */
 				tran[U_SHEARXY] = row[0].dot(row[1]);

				row[1] = row[1].combine(1.0, row[0], -tran[U_SHEARXY]);

 				/* Now, compute Y scale and normalize 2nd row. */
 				tran[U_SCALEY] = row[1].length();
 				row[1].normalize();
 				tran[U_SHEARXY] /= tran[U_SCALEY];

 				/* Compute XZ and YZ shears, orthogonalize 3rd row. */
 				tran[U_SHEARXZ] = row[0].dot(row[2]);
				row[2] = row[2].combine(1.0, row[0], -tran[U_SHEARXZ]);

				tran[U_SHEARYZ] = row[1].dot(row[2]);
				row[2] = row[2].combine(1.0, row[1], -tran[U_SHEARYZ]);

 			/* Next, get Z scale and normalize 3rd row. */
 			tran[U_SCALEZ] = row[2].length();
 			row[2].normalize();
 			tran[U_SHEARXZ] /= tran[U_SCALEZ];
 			tran[U_SHEARYZ] /= tran[U_SCALEZ];

 			/* At this point, the matrix (in rows[]) is orthonormal.
 			* Check for a coordinate system flip.  If the determinant
 			* is -1, then negate the matrix and the scaling factors.
 			*/
 			pdum3 = row[1];
			pdum3.cross(row[2]);

			if ( row[0].dot(pdum3) < 0 )
 				for ( i = 0; i < 3; i++ ) {
 					tran[U_SCALEX+i] *= -1;
 					row[i](0,0) *= -1;
 					row[i](1,0) *= -1;
 					row[i](2,0) *= -1;
 				}

 			/* Now, get the rotations out, as described in the gem. */
 			tran[U_ROTATEY] = asin(-row[0].z());
 			if ( cos(tran[U_ROTATEY]) != 0 ) {
 				tran[U_ROTATEX] = atan2(row[1].z(), row[2].z());
 				tran[U_ROTATEZ] = atan2(row[0].y(), row[0].x());
 			} else {
 				tran[U_ROTATEX] = atan2(-row[2].x(), row[1].y());
 				tran[U_ROTATEZ] = 0;
 			}
 			/* All done! */
 			return 1;

		}
				// Debugging functions

#ifdef _MSC_VER
inline		void		debugTrace() const
				{
					for (unsigned int i = 0; i < N; i++)
					{
						char	temp[90];
						strcpy(temp, "[");
						for (unsigned int j = 0; j < M; j++)
						{
							char	t[90];
							sprintf(t, "%12.5f", (*this)(i,j));
							strcat(temp, t);
						}
						strcat(temp, " ]\n");
						TRACE(temp);
					}
					TRACE("\n");
				}
#endif

#ifdef _H_LOGGER
inline		void		debugLog(const char *title) const
				{
					LOGBLOCK(title);
					for (unsigned int i = 0; i < N; i++)
					{
						char	temp[90];
						strcpy(temp, "[");
						for (unsigned int j = 0; j < M; j++)
						{
							char	t[90];
							sprintf(t, "%12.5f", (*this)(i,j));
							strcat(temp, t);
						}
						strcat(temp, " ]");
						LOG(temp);
					}
				}
#endif

/*
inline		void		debugOut(std::ostream &s) const
				{
					for (unsigned int i = 0; i < N; i++)
					{
						s << "[";
						for (unsigned int j = 0; j < M; j++)
						{
							char	t[90];
							sprintf(t, "%12.5f", (*this)(i,j));
							s << t;
						}
						s << " ]" << endl;
					}
					s << endl;
				}
*/
private:
		T		_data[N*M];
};

// --------------------------------------------------------------------------------------------------------------------------------
// Convenience types - Most common uses
// --------------------------------------------------------------------------------------------------------------------------------

typedef	matrix<3, 3> mmatrix3;
typedef	matrix<4, 4> mmatrix4;
typedef	matrix<3, 1> mvector3;
typedef	matrix<2, 1> mvector2;
typedef	matrix<4, 1> mvector4;
typedef	matrix<2, 1> mpoint2;
typedef	matrix<3, 1> mpoint3;
typedef	matrix<4, 1> mpoint4;

typedef	matrix<3, 3, double> mmatrix3d;
typedef	matrix<4, 4, double> mmatrix4d;
typedef	matrix<2, 1, double> mvector2d;
typedef	matrix<3, 1, double> mvector3d;
typedef	matrix<4, 1, double> mvector4d;
typedef	matrix<2, 1, double> mpoint2d;
typedef	matrix<3, 1, double> mpoint3d;
typedef	matrix<4, 1, double> mpoint4d;
// --------------------------------------------------------------------------------------------------------------------------------
// Mixed-mode global overrides
// --------------------------------------------------------------------------------------------------------------------------------

template <unsigned int N, unsigned int M, class T>
inline	const	matrix<N, M, T>	operator *(const T &value, const matrix<N, M, T> &m) {return m * value;}

template <unsigned int N, unsigned int M, class T>
inline	const	matrix<N, M, T>	operator /(const T &value, const matrix<N, M, T> &m) {return m.inverseDivide(value);}

template <unsigned int N, unsigned int M, class T>
inline	const	matrix<N, M, T>	operator +(const T &value, const matrix<N, M, T> &m) {return m + value;}

template <unsigned int N, unsigned int M, class T>
inline	const	matrix<N, M, T>	operator -(const T &value, const matrix<N, M, T> &m) {return m.inverseSubtract(value);}

#endif // _H_VMATH
// ---------------------------------------------------------------------------------------------------------------------------------
// VMath.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------

