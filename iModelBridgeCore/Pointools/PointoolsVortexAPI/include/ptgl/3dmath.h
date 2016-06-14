// this file uses Doxygen comment blocks for automatic extraction of source code documentation.

/*!\file
 \brief Contains some useful math helpers for common 3D graphics entities.

 In this file are implemented some mathematical entities often used in 3D graphics (vecors, points, etc.) plus some useful constants, macros and definitions
 \version 0.1
 \date 12/06/99
 \author Alessandro Falappa
*/

#ifndef _3D_MATH_H
#define _3D_MATH_H

#include <assert.h>
#ifdef __INTEL_COMPILER
#include <mathimf.h>
#else
#include <math.h>
#endif

#ifdef HIGH_PRECISION

//! The base type for all the math helpers
typedef double real;
//! the treshold for comparisons with zero, mainly used to avoid division by zero errors
const real epsilon=1e-12;
//! defined when high precision is requested
#define REAL_IS_DOUBLE

#else

#if defined (BENTLEY_WIN32)     //NEEDS_WORK_VORTEX_DGNDB - Pragmas unrecognized on iOS
    // WARNING: these pragmas below could be MSVC compiler specific
    #pragma warning( push )// memorize the warning status
    #pragma warning( disable : 4305 )// disable "initializing : truncation from 'const double' to 'float'" warning
    #pragma warning( disable : 4244 )// disable "double to float conversion possible loss of data" warning
    #pragma warning( disable : 4136 )// disable "conversion between different floating-point types" warning
    #pragma warning( disable : 4309 )// disable " 'conversion' : truncation of constant value" warning
    #pragma warning( disable : 4051 )// disable " 'type conversion' ; possible loss of data" warning
#endif

//! The base type for all the math helpers
typedef float real;
//! the treshold for comparisons with zero, mainly used to avoid division by zero errors
const real epsilon=1e-7;
//! defined when high precision is not requested
#define REAL_IS_FLOAT

#endif

//=============================================================================
//=============================================================================

//!A gl_vector class.
/*!
The gl_vector class incapsulates a classic C++ gl_vector of three real values and treats them as a 3 dimensional mathematical vectors.

The most common operations between mathematical vectors (and some which involves scalars too) are defined.
*/
class gl_vector
{
private:
	static int counter;//!< counts how many gl_vector objects are present
	real vec[3];//!< the actual gl_vector

public:
// operators
	gl_vector();//!< default constructor
	gl_vector(const real& x,const real& y,const real& z);//!< constructs a gl_vector from three values
	gl_vector(gl_vector& from, gl_vector& to);//!< constructs a gl_vector from two other vectors
	gl_vector(const gl_vector& other);//!< the copy constructor
	~gl_vector();//!< the distructor
	real& x();//!< accessor for the x component (can be used as l-value too)
	real& y();//!< accessor for the y component (can be used as l-value too)
	real& z();//!< accessor for the z component (can be used as l-value too)
	real x() const;//!< returns the x component (r-value only)
	real y() const;//!< returns the y component (r-value only)
	real z() const;//!< returns the z component (r-value only)
	gl_vector& operator=(const gl_vector& other);//!< the assignment
	gl_vector& operator+=(const gl_vector& other);//!< the sum & assign
	gl_vector& operator-=(const gl_vector& other);//!< the subtract & assign
	gl_vector& operator*=(const real& fact);//!< the short multiply by a scalar factor & assign
	gl_vector& operator/=(const real& fact);//!< the short divide by a scalar factor & assign
	real& operator[](const int& index);//!< an "access like a C++ gl_vector"
#ifdef HIGH_PRECISION
	operator double*();//!< the conversion to a 3 double elements C++ gl_vector
#else
	operator float*();//!< the conversion to a 3 float elements C++ gl_vector
#endif
//	operator char*();//!< the conversion of the gl_vector into a textual form (null terminated string)
	real normalize();//!< normalize the gl_vector
	gl_vector normalized() const;//!< normalized copy of the gl_vector
	real length() const;//!< get the length of the gl_vector
	real length2() const;//!< get the squared length of the gl_vector
	void EpsilonCorrect(const gl_vector& v);//!< if the gl_vector is almost equal to the origin substitute it with v
// ststic functions
	static int howMany();//!< returns how many gl_vector objects exists

// friend functions
	friend int operator==(const gl_vector& v1,const gl_vector& v2);//!< the equality operator
	friend int operator!=(const gl_vector& v1,const gl_vector& v2);//!< the inequality operator
	friend gl_vector operator+(const gl_vector& v1,const gl_vector& v2);//!< the sum
	friend gl_vector operator-(const gl_vector& v1,const gl_vector& v2);//!< the difference
	friend gl_vector operator-(const gl_vector& v1);//!< the negation
	friend real operator*(const gl_vector& v1,const gl_vector& v2);//!< the gl_vector dot product
	friend gl_vector operator^(const gl_vector& v1,const gl_vector& v2);//!< the gl_vector cross product
	friend gl_vector operator*(const gl_vector& v,const real& fact);//!< the multiply a gl_vector by a scalar factor
	friend gl_vector operator*(const real& fact,const gl_vector& v);//!< the multiply a scalar factor by a gl_vector
	friend gl_vector operator/(const gl_vector& v,const real& fact);//!< the divide a gl_vector by a scalar factor
	friend gl_vector Bisect(gl_vector v0,gl_vector v1);//!< returns the unit gl_vector which halves the arc between v0 and v1
	friend void clamp(gl_vector& vec,const real& low,const real& high);//!< clamps all the gl_vector components between the given tresholds
/*	friend std::ostream& operator<<(std::ostream& os,const gl_vector& vect);//!< the print gl_vector to C++ stream
#ifdef _AFXDLL // see if we are using MFC...
#ifdef _DEBUG		//.. and if we are in a debug build
	friend CDumpContext& operator<<(CDumpContext& cd,const gl_vector& vect);//!< the print gl_vector to MSVC++ debug console
#endif
#endif
	*/
};
//-----------------------------------------------------------------------------
// inlines

inline gl_vector::gl_vector()
{
	vec[0]=vec[1]=vec[2]=0.0;
	counter++;
}

inline gl_vector::gl_vector(const real& x,const real& y,const real& z)
{
	vec[0]=x;
	vec[1]=y;
	vec[2]=z;
	counter++;
}

/*!
This function constructs the gl_vector which goes from the gl_vector \e from to the gl_vector \e to.
It is useful when a gl_vector has to be constructed from the difference of two other vectors.
\param from the first gl_vector
\param to the second gl_vector
*/
inline gl_vector::gl_vector(gl_vector& from, gl_vector& to)
{
	vec[0]=to.vec[0]-from.vec[0];
	vec[1]=to.vec[1]-from.vec[1];
	vec[2]=to.vec[2]-from.vec[2];
	counter++;
}

inline gl_vector::gl_vector(const gl_vector& other)
{
	*this=other;
	counter++;
}

inline gl_vector::~gl_vector()
{
	counter--;
}

inline real& gl_vector::x()
{
	return vec[0];
}

inline real& gl_vector::y()
{
	return vec[1];
}

inline real& gl_vector::z()
{
	return vec[2];
}

inline real gl_vector::x() const
{
	return vec[0];
}

inline real gl_vector::y() const
{
	return vec[1];
}

inline real gl_vector::z() const
{
	return vec[2];
}

inline gl_vector& gl_vector::operator=(const gl_vector& other)
{
	//check for 'a=a' case
	if (this==&other) return *this;
	vec[0]=other.vec[0];
	vec[1]=other.vec[1];
	vec[2]=other.vec[2];
	return *this;
}

inline gl_vector& gl_vector::operator+=(const gl_vector& other)
{
	vec[0]+=other.vec[0];
	vec[1]+=other.vec[1];
	vec[2]+=other.vec[2];
	return *this;
}

inline gl_vector& gl_vector::operator-=(const gl_vector& other)
{
	vec[0]-=other.vec[0];
	vec[1]-=other.vec[1];
	vec[2]-=other.vec[2];
	return *this;
}

inline gl_vector& gl_vector::operator*=(const real& fact)
{
	vec[0]*=fact;
	vec[1]*=fact;
	vec[2]*=fact;
	return *this;
}

inline gl_vector& gl_vector::operator/=(const real& fact)
{
	assert(fabs(fact) >= epsilon);
	vec[0]/=fact;
	vec[1]/=fact;
	vec[2]/=fact;
	return *this;
}

/*!
This operator redirects the access to the internal gl_vector. It does make a range check on the index in debug builds trough the ANSI assert function.
It can be used on both sides of an assignment.
\return a reference to the requested element
\param index the index which should be one of 0,1,2
*/
inline real& gl_vector::operator[](const int& index)
{
	assert(index>=0 && index<=2);
	return vec[index];
}

inline real gl_vector::length2() const
{
	return (*this)*(*this);
}

inline real gl_vector::length() const
{
	return sqrt(this->length2());
}

/*!
The counting of the gl_vector objects is realized trough a static counter variable.
\return the number of gl_vector objects in memory
*/
inline int gl_vector::howMany()
{
	return counter;
}

/*!
this conversion operator allows to use a gl_vector in places of a classic real[3]
*/
/*
inline gl_vector::operator real*()
{
	return (real*)vec;
}
*/
#ifdef HIGH_PRECISION

/*!
this conversion operator allows to use a gl_vector in places of a classic double[3]
*/
inline gl_vector::operator double*()
{
	return (double*)vec;
}

#else

/*!
this conversion operator allows to use a gl_vector in place of a classic float[3]
*/
inline gl_vector::operator float*()
{
	return (float*)vec;
}

#endif

//-----------------------------------------------------------------------------
// useful constants (declaration)

extern const gl_vector ORIGIN;
extern const gl_vector X_AXIS;
extern const gl_vector Y_AXIS;
extern const gl_vector Z_AXIS;

//=============================================================================
//=============================================================================

/*!
A transformation matrix class.

The matrix class groups sixteen real values an treats them as a 4x4 matrix. Standard
C++ matrices (2 dimensional vectors) are stored by row, since for graphical applications
a column major order is preferable the access indices are internally swapped.

The most common operators between matrices (and some which involves scalars and vectors too) are defined.
*/
class gl_tmatrix
{
private:
	static int counter;//!< counts how many matrices objects are present
	real mat[4][4];//!< the matrix data
public:
	//! Espresses how to store a gl_tmatrix in a single 16 elements gl_vector, by column or by row
	enum ordermode
	{
		COLUMN,//!< column major order
		ROW//!< row major order
	};
	gl_tmatrix();//!< default constructor
	gl_tmatrix(const real& val);//!< constructs a gl_tmatrix and fills it with a value
	gl_tmatrix(const real gl_vector[16],ordermode ord=COLUMN);//!< constructs a gl_tmatrix from a gl_vector which contains a 4x4 matrix in row major or column major order
	gl_tmatrix(const gl_tmatrix& other);//!< the copy constructor
	~gl_tmatrix();//!< the distructor
	gl_tmatrix& operator-();//!< negation
	gl_tmatrix& operator=(const gl_tmatrix& other);//!< assignment
	gl_tmatrix& operator+=(const gl_tmatrix& other);//!< sum & assign
	gl_tmatrix& operator-=(const gl_tmatrix& other);//!< subtract & assign
	gl_tmatrix& operator*=(const gl_tmatrix& other);//!< multiply by a gl_tmatrix & assign
	gl_tmatrix& operator*=(const real& fact);//!< multiply by a scalar factor & assign
	gl_tmatrix& operator/=(const real& fact);//!< divide by a scalar factor & assign
	real& operator()(const int& row,const int& col);//!< an "access like a C++ gl_tmatrix"
#ifdef HIGH_PRECISION
	operator double*();//!< conversion to a 16 doubles C++ gl_vector (column major order)
#else
	operator float*();//!< conversion to a 16 floats C++ gl_vector (column major order)
#endif
	void loadIdentity();//!< fills the matrix with the identity matrix
//	operator char*();//!< conversion of the gl_tmatrix into a textual form (null terminated string)
//static functions
	static int howMany();//!< returns how many gl_tmatrix objects exists
// friend functions
	friend int operator==(const gl_tmatrix& t1,const gl_tmatrix& t2);//!< the equality operator
	friend int operator!=(const gl_tmatrix& t1,const gl_tmatrix& t2);//!< the inequality operator
	friend gl_tmatrix operator+(const gl_tmatrix& t1,const gl_tmatrix& t2);//!< the sum
	friend gl_tmatrix operator-(const gl_tmatrix& t1,const gl_tmatrix& t2);//!< the difference
	friend gl_tmatrix operator*(const gl_tmatrix& t1,const gl_tmatrix& t2);//!< gl_tmatrix product
	friend gl_tmatrix operator*(const gl_tmatrix& tmat,const real& fact);//!< multiply a gl_tmatrix by a scalar factor
	friend gl_tmatrix operator*(const real& fact,const gl_tmatrix& tmat);//!< multiply a scalar factor by a gl_tmatrix
	friend gl_tmatrix operator/(const gl_tmatrix& tmat,const real& fact);//!< divide a gl_tmatrix by a scalar factor
/*	friend std::ostream& operator<<(std::ostream& os,const gl_tmatrix& m);//!< print gl_tmatrix to C++ stream
#ifdef _AFXDLL // see if we are using MFC
#ifdef _DEBUG
	friend CDumpContext& operator<<(CDumpContext& cd,const gl_tmatrix& m);//!< print gl_tmatrix to MSVC++ debug console
#endif
#endif
	*/
};

//-----------------------------------------------------------------------------
// inlines

inline gl_tmatrix::gl_tmatrix(const gl_tmatrix& other)
{
	*this=other;
	counter++;
}

inline gl_tmatrix::~gl_tmatrix()
{
	counter--;
}

/*!
The counting of the gl_tmatrix objects is realized trough a static counter variable.
\return the number of gl_tmatrix objects in memory
*/
inline int gl_tmatrix::howMany()
{
	return counter;
}

/*!
This operator redirects the access to the internal matrix. It does make a range
check on the index in debug builds trough the ANSI assert function.
It can be used on both sides of an assignment.
\return a reference to the requested element
\param row the row index which should be one of 0,1,2,3
\param col the column index which should be one of 0,1,2,3
\date 18/06/99
*/
inline real& gl_tmatrix::operator()(const int& row,const int& col)
{
	assert(row>=0 && row<=3);
	assert(col>=0 && col<=3);
	return mat[col][row];// swap indices to store by column
}

#ifdef HIGH_PRECISION

/*!
this conversion operator allows to use a gl_tmatrix in places where a column major
order gl_vector of 16 double elements is requested (e.g. the OpenGL functions for
retrieving/setting the modelview or projection matrix).
*/
inline gl_tmatrix::operator double*()
{
	return (double*)mat;
}

#else

/*!
this conversion operator allows to use a gl_tmatrix in places where a column major
order gl_vector of 16 float elements is requested (e.g. the OpenGL functions for
retrieving/setting the modelview or projection matrix).
*/
inline gl_tmatrix::operator float*()
{
	return (float*)mat;
}

#endif

//-----------------------------------------------------------------------------
// useful constants


//=============================================================================
//=============================================================================

/*!
A generic gl_quaternion class.
The gl_quaternion uses internally the (s,<B>v</B>) representation, where s is a scalar and <B>v</B> a gl_vector.
The most common operations from gl_quaternion calculus are implemented.

Some internet references are:
<UL>
<LI>http://www.cs.berkley.edu/~laura/cs184/quat/gl_quaternion.html
<LI>http://www.astro.virginia.edu/~eww6n/math/Quaternion.html
<LI>http://forum.swarthmore.edu/dr.math/problems/prasad2.17.96.html
<LI>http://graphics.cs.ucdavis.edu/GraphicsNotes/Quaternions/Quaternions.html
</UL>

A paper by Ken Shoemake (the quaternions "inventor") is available at<BR>
<UL>
<LI>ftp://ftp.cis.upenn.edu/pub/graphics/shoemake/
</UL>
*/
class gl_quaternion
{
private:
	static int counter;//!< counts how many gl_quaternion objects are present
protected:
	real s;//!< the scalar part of a gl_quaternion
	gl_vector v;//!< the gl_vector part of a gl_quaternion
public:
	gl_quaternion();//!< default constructor
	gl_quaternion(const real& scal, const gl_vector& vec);//!< constructs a gl_quaternion from the scalar and gl_vector components
	gl_quaternion(const real& s1,const real& s2,const real& s3,const real& s4);//!< constructs a gl_quaternion from four real values
	gl_quaternion(const gl_quaternion& other);//!< the copy constructor
	virtual ~gl_quaternion();//!< the distructor
	real& x();//!< accessor for the x component of the gl_vector part (can be used as l-value too)
	real& y();//!< accessor for the y component of the gl_vector part (can be used as l-value too)
	real& z();//!< accessor for the z component of the gl_vector part (can be used as l-value too)
	real& w();//!< accessor for the scalar part (can be used as l-value too)
	real& scalarPart();//!< other accessor for the scalar component (can be used as l-value too)
	gl_vector& vectorPart();//!< accessor for the gl_vector part (can be used as l-value too)
	gl_quaternion& operator=(const gl_quaternion& other);//!< assignment
	gl_quaternion& operator+=(const gl_quaternion& other);//!< sum & assign
	gl_quaternion& operator-=(const gl_quaternion& other);//!< subtract & assign
	gl_quaternion& operator*=(const gl_quaternion& other);//!< multiply by a gl_quaternion & assign
	gl_quaternion& operator/=(const gl_quaternion& other);//!< divide by a gl_quaternion & assign
	gl_quaternion& operator*=(const real& fact);//!< multiply by a scalar factor & assign
	gl_quaternion& operator/=(const real& fact);//!< divide by a scalar factor & assign
//	operator float*();//!< conversion to a 4 elements C++ gl_vector
//	operator char*();//!< conversion of the gl_quaternion into a textual form (null terminated string)
	real normalize();//!< normalize the gl_quaternion
	gl_quaternion normalized() const;//!< normalized copy of the gl_quaternion
	void conjugate();//!< conjugate of the gl_quaternion
	gl_quaternion conjugated() const;//!< conjugated copy of the gl_quaternion
	real inverse();//!< inverse of the gl_quaternion
	gl_quaternion inversed() const;//!< inversed copy of the gl_quaternion
	real length() const;//!< get the length of the gl_quaternion
	real norm() const;//!< get the norm (similar to the squared length) of the gl_quaternion
	virtual gl_tmatrix getRotMatrix() const;//<! constructs a rotation matrix from the gl_quaternion
// static functions
	static int howMany();//!< returns how many gl_quaternion objects exists
// friend functions
	friend int operator==(const gl_quaternion& q1,const gl_quaternion& q2);//!< the equality operator
	friend int operator!=(const gl_quaternion& q1,const gl_quaternion& q2);//!< the inequality operator
	friend gl_quaternion operator+(const gl_quaternion& q1,const gl_quaternion& q2);//!< the sum
	friend gl_quaternion operator-(const gl_quaternion& q1,const gl_quaternion& q2);//!< the difference
	friend gl_quaternion operator-(const gl_quaternion& q1);//!< negation
	friend gl_quaternion operator*(const gl_quaternion& q1,const gl_quaternion& q2);//!< gl_quaternion product
	friend gl_quaternion operator*(const gl_quaternion& q,const real& s);//!< multiply a gl_quaternion by a scalar
	friend gl_quaternion operator*(const real& s,const gl_quaternion& q);//!< multiply a scalar by a gl_quaternion
	friend gl_quaternion operator/(const gl_quaternion& q,const real& s);//!< divide a gl_quaternion by a scalar factor
	friend gl_quaternion operator/(const gl_quaternion& q1,const gl_quaternion& q2);//!< divide a gl_quaternion by a gl_quaternion
/*	friend std::ostream& operator<<(std::ostream& os,const gl_quaternion& q);//!< print gl_quaternion to C++ stream
#ifdef _AFXDLL // see if we are using MFC
#ifdef _DEBUG
	friend CDumpContext& operator<<(CDumpContext& ad,const gl_quaternion& q);//!< print gl_quaternion to MSVC++ debug console
#endif
#endif
	*/
};

//-----------------------------------------------------------------------------
// inlines

inline gl_quaternion::gl_quaternion()
{
	s=0.0;
	counter++;
};

/*!
This constructor assumes an (s,\b v) form so interprets the real calues as w,x,y,z respectively.
Another common representation for quaternions, the "homogeneous" one, is x,y,x,w ordered.
\param s1 the scalar part
\param s2 the x component of the gl_vector part
\param s3 the y component of the gl_vector part
\param s4 the z component of the gl_vector part
\date 15/06/99
*/
inline gl_quaternion::gl_quaternion (const real& s1,const real& s2,const real& s3,const real& s4)
:s(s1),v(s2,s3,s4)
{
	counter++;
}

inline gl_quaternion::gl_quaternion (const real& scal, const gl_vector& vec)
:s(scal),v(vec)
{
	s=scal;
	counter++;
}

inline gl_quaternion::~gl_quaternion()
{
	counter--;
}

inline gl_quaternion::gl_quaternion(const gl_quaternion& other)
{
	*this=other;
	counter++;
}

/*!
The counting of the gl_quaternion objects is realized trough a static counter variable.
\return the number of gl_quaternion objects in memory
*/
inline int gl_quaternion::howMany()
{
	return counter;
}

inline real& gl_quaternion::x()
{
	return v.x();
}

inline real& gl_quaternion::y()
{
	return v.y();
}

inline real& gl_quaternion::z()
{
	return v.z();
}

inline real& gl_quaternion::w()
{
	return s;
}

inline real& gl_quaternion::scalarPart()
{
	return s;
}

inline gl_vector& gl_quaternion::vectorPart()
{
	return v;
}

inline gl_quaternion& gl_quaternion::operator=(const gl_quaternion& other)
{
	//check for 'a=a' case
	if (this==&other) return *this;
	s=other.s;
	v=other.v;
	return *this;
}

inline gl_quaternion& gl_quaternion::operator+=(const gl_quaternion& other)
{
	s+=other.s;
	v+=other.v;
	return *this;
}

inline gl_quaternion& gl_quaternion::operator-=(const gl_quaternion& other)
{
	s-=other.s;
	v-=other.v;
	return *this;
}

inline gl_quaternion& gl_quaternion::operator*=(const real& fact)
{
	s*=fact;
	v*=fact;
	return *this;
}

inline gl_quaternion& gl_quaternion::operator/=(const real& fact)
{
	assert(fabs(fact) >= epsilon);
	s/=fact;
	v/=fact;
	return *this;
}

inline real gl_quaternion::length() const
{
	return sqrt(norm());
}

inline void gl_quaternion::conjugate()
{
	v=-v;
}

inline gl_quaternion gl_quaternion::conjugated() const
{
	return gl_quaternion(s,-v);
}

/*!
This function check if all elements of \e v1 are equal to the corresponding elements of \e v2.
*/
inline int operator==(const gl_vector& v1,const gl_vector& v2)
{
	if(v1.vec[0]==v2.vec[0] && v1.vec[1]==v2.vec[1] && v1.vec[2]==v2.vec[2]) return 1;
	else return 0;
}

/*!
This function check if some of the elements of \e v1 differs from the corresponding elements of \e v2.
*/
inline int operator!=(const gl_vector& v1,const gl_vector& v2)
{
	if(v1.vec[0]==v2.vec[0] && v1.vec[1]==v2.vec[1] && v1.vec[2]==v2.vec[2]) return 0;
	else return 1;
}


//=============================================================================
//=============================================================================

#if defined (BENTLEY_WIN32)     //NEEDS_WORK_VORTEX_DGNDB - Pragmas unrecognized on iOS
    #pragma warning(disable:4263)
    #pragma warning(disable:4264)
#endif

/*!
A unit lenght gl_quaternion class.

The gl_unitquaternion class manages quaternions of unit length, such quaternions
can be used to represent arbitrary rotations. To mantain the unit lenght property
along gl_quaternion calculus addition and subtraction as well as multiplication and
division by scalars are not allowed (they're made private or they're overloaded
by "trap" functions).

Actually only a few member functions of the base class are overloaded since
the gl_quaternion calculus apply with no modification to unit quaternions.
*/
class gl_unitquaternion : public gl_quaternion
{
private:
	gl_unitquaternion& operator+=(const gl_unitquaternion& other);//!< sum & assign is not allowed
	gl_unitquaternion& operator-=(const gl_unitquaternion& other);//!< subtract & assign is not allowed
	gl_unitquaternion& operator/=(const real& fact);//!< divide by a scalar factor & assign is not allowed
public:
	gl_unitquaternion& operator*=(const real& fact);//!< multiply by a scalar factor & assign is not allowed
	gl_unitquaternion(const gl_vector& v, const real& s);//!< constructs a gl_unitquaternion assigning directly the scalar and gl_vector parts
public:
	real x() const;//!< accessor for the x component of the gl_vector part
	real y() const;//!< accessor for the y component of the gl_vector part
	real z() const;//!< accessor for the z component of the gl_vector part
	real w() const;//!< accessor for the scalar part
	real scalarPart() const;//!< other accessor for the scalar component
	gl_vector vectorPart() const;//!< accessor for the gl_vector part
	gl_unitquaternion();//!< default constructor
	gl_unitquaternion(const real& angle, const gl_vector& axis);//!< constructs a gl_unitquaternion representing a rotation of angle radiants about axis
	gl_unitquaternion(const gl_quaternion& q);//!< constructs a gl_unitquaternion from a generic one (conversion by normalizing)
	~gl_unitquaternion();//!< the distructor
	gl_unitquaternion inversed() const;//!< inversed copy of the gl_unitquaternion
	void inverse();//!< inverse of the gl_unitquaternion
	gl_tmatrix getRotMatrix() const;//<! constructs a rotation matrix from the gl_quaternion
	void getVectorsOnSphere(gl_vector& vfrom,gl_vector& vto);//<! converts a gl_unitquaternion to two vectors on a unit sphere (the extremes of a rotation)
	gl_unitquaternion& operator*=(const gl_unitquaternion& other);//!< multiply by another gl_unitquaternion & assign is not allowed
// friend functions
	friend gl_unitquaternion operator+(const gl_unitquaternion& q1,const gl_unitquaternion& q2);//!< the sum is not allowed
	friend gl_unitquaternion operator-(const gl_unitquaternion& q1,const gl_unitquaternion& q2);//!< the difference is not allowed
	friend gl_unitquaternion operator*(const gl_unitquaternion& q,const real& s);//!< multiply a gl_unitquaternion by a scalar is not allowed
	friend gl_unitquaternion operator*(const real& s,const gl_unitquaternion& q);//!< multiply a scalar by a gl_unitquaternion is not allowed
	friend gl_unitquaternion operator/(const gl_unitquaternion& q,const real& s);//!< divide a gl_unitquaternion by a scalar factor is not allowed
};

//-----------------------------------------------------------------------------
// inlines

inline gl_unitquaternion::gl_unitquaternion()
:gl_quaternion(1,0,0,0)
{
};


inline gl_unitquaternion::gl_unitquaternion (const gl_quaternion& q)
:gl_quaternion(q)
{
	normalize();
}


/*!
This constructor has a different meaning from the same in the base class. Here
the gl_vector means an axis of rotation while the real means the angle to rotate
about the axis.

\b NOTE: the angle and axis gl_vector are not directly assigned to the real part
and the gl_vector part, respectively, of the gl_quaternion. The unit gl_quaternion (<I>s</I>,<B>v</B>)
represents a rotation of \b angle radians about the axis \b a if:

\e s = cos(\e angle / 2)<BR>
\b v = \b a * sin( \e angle / 2)
\param angle the rotation angle
\param axis the axis of rotation
\date 18/06/99
*/
inline gl_unitquaternion::gl_unitquaternion (const real& angle, const gl_vector& axis)
:gl_quaternion( cos(angle/2), sin(angle/2)*axis.normalized() )
{
}

/*!
This constructor has been introduced exclusively to make the inversed function
more efficient by avoiding too many gl_unitquaternion copies and conversions to
quaternions.
*/
inline gl_unitquaternion::gl_unitquaternion (const gl_vector& v, const real& s)
:gl_quaternion( s, v )
{
}

inline gl_unitquaternion::~gl_unitquaternion()
{
}

/*!
For unitquaternions the inverse equals the conjugate (which is simpler to calculate)
This function doesn't modifies the gl_quaternion upon which has been called, it returns a new gl_quaternion instead.
\return the normalized copy of the gl_quaternion
\date 18/06/99
*/
inline gl_unitquaternion gl_unitquaternion::inversed() const
{
	return gl_unitquaternion(-v,s);
}

/*!
For unitquaternions the inverse equals the conjugate (which is simpler
to calculate). The function doesn't call the base conjugate function to
avoid an expensive gl_quaternion to gl_unitquaternion conversion.
This function modifies the gl_quaternion upon which has been called.
*/
inline void gl_unitquaternion::inverse()
{
	v=-v;
}

/*!
This function overrides the same in base class to prevent the use as an l-value
(that is to modify the x component of the gl_vector part).
*/
inline real gl_unitquaternion::x() const
{
	return ((gl_vector&)v).x();// explicit cast to use the non const gl_vector x() function
}

/*!
This function overrides the same in base class to prevent the use as an l-value
(that is to modify the y component of the gl_vector part).
*/
inline real gl_unitquaternion::y() const
{
	return ((gl_vector&)v).y();// explicit cast to use the non const gl_vector y() function
}

/*!
This function overrides the same in base class to prevent the use as an l-value
(that is to modify the z component of the gl_vector part).
*/
inline real gl_unitquaternion::z() const
{
	return ((gl_vector&)v).z();// explicit cast to use the non const gl_vector z() function
}

/*!
This function overrides the same in base class to prevent the use as an l-value
(that is to modify the scalar part).
*/
inline real gl_unitquaternion::w() const
{
	return s;
}

/*!
This function overrides the same in base class to prevent the use as an l-value
(that is to modify the scalar part).
*/
inline real gl_unitquaternion::scalarPart() const
{
	return s;
}

/*!
This function overrides the same in base class to prevent the use as an l-value
(that is to modify the gl_vector part).
*/
inline gl_vector gl_unitquaternion::vectorPart() const
{
	return v;
}


//-----------------------------------------------------------------------------
// useful constants

//=============================================================================
//=============================================================================

/* inserire qui alcune macro o funzioni globali come ad esempio:
	- clamping
	- conversione angoli gradi<->radianti
*/

/*!
limits a value in a range, modifying it.
\param val the value to clamp
\param low the lower treshold
\param high the higher treshold
*/
inline void clamp(real& val,const real& low,const real& high)
{
	if(val<low) val=low;
	if(val>high) val=high;
}

/*!
limits a value in a range, returning the clamped value.
\return the clamped value
\param val the value to clamp
\param low the lower treshold
\param high the higher treshold
*/
inline real clamped(const real& val,const real& low,const real& high)
{
	if(val<low) return low;
	else if(val>high) return high;
		else return val;
}


/*!
limits the gl_vector components in a range, modifying it.
\param theVec the gl_vector to clamp
\param low the lower treshold
\param high the higher treshold
*/
inline void clamp(gl_vector& theVec,const real& low,const real& high)
{
	clamp(theVec.vec[0],low,high);
	clamp(theVec.vec[1],low,high);
	clamp(theVec.vec[2],low,high);
}

/*!
\short returns \e angle expressed in degrees.
\return the angle expresses in radians
\param angle the angle value
\author Alessandro Falappa
*/
inline real RadToDeg(const real& angle)
{
	return (real)(angle*57.29577951308);
}

/*!
\short returns \e angle expressed in radians.
\return the angle expresses in degrees
\param angle the angle value
*/
inline real DegToRad(const real& angle)
{
	return (real) (angle*0.01745329251994);
}

/*!
\short converts radiants to degrees.
This function modify its argument.
\param angle the angle to be converted
*/
inline void ConvertToDeg(real& angle)
{
	angle*=57.29577951308;
}

/*!
\short converts degrees to radiants.
This function modify its argument.
\param angle the angle to be converted
*/
inline void ConvertToRad(real& angle)
{
	angle*=0.01745329251994;
}

/*!
\short absolute value function which executes a simple test
This function executes a simple test on \e val negativity returning the
opposite if true. Such a test can be faster than the call to the \e fabs
library routine
\return the absolute value of \e val
*/
inline real simpleabs(const real &val)
{
	return val>0?val:-val;
}
//! the greek pi constant
extern const real G_PI;

//! greek pi / 2
extern const real G_HALF_PI;

//!2 * greek pi
extern const real G_DOUBLE_PI;

#ifdef _AFXDLL
#undef assert
//!maps the assert function to the MFC ASSERT macro
#define assert(exp) ASSERT(exp)
#endif

#ifndef HIGH_PRECISION
    #if defined (BENTLEY_WIN32)     //NEEDS_WORK_VORTEX_DGNDB - Pragmas unrecognized on iOS
        // this below could be MSVC compiler specific
        #pragma warning( pop )// reset the warning status
    #endif
#endif

#endif // _3D_MATH_H