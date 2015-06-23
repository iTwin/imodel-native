// this file uses Doxygen comment blocks for automatic extraction of source code documentation.

/*!\file
 Implementation for 3dmath classes
 \version 0.1
 \date 15/06/99
 \author Alessandro Falappa
*/

#include <ptgl\3dmath.h>

#ifdef REAL_IS_FLOAT
// WARNING: these pragmas below could be MSVC compiler specific
#pragma warning(push)// memorize the warning status
#pragma warning(disable:4305)// disable "initializing : truncation from 'const double' to 'float'" warning
#pragma warning(disable:4244)// disable "double to float conversion possible loss of data" warning
#endif

//-----------------------------------------------------------------------------
// static members

int gl_vector::counter=0;// global counter initialization
int gl_tmatrix::counter=0;// global counter initialization
int gl_quaternion::counter=0;// global counter initialization

//-----------------------------------------------------------------------------
// useful constants (definition)

//!the gl_vector representing the origin
const gl_vector ORIGIN(0,0,0);
//!the unit gl_vector associated to the x axys
const gl_vector X_AXIS(1,0,0);
//!the unit gl_vector associated to the y axys
const gl_vector Y_AXIS(0,1,0);
//!the unit gl_vector associated to the z axys
const gl_vector Z_AXIS(0,0,1);
//!the greek pi constant
const real G_PI=3.14159265359;
//! greek pi / 2
const real G_HALF_PI= 1.570796326795;
//!2 * greek pi
const real G_DOUBLE_PI= 6.28318530718;

//-----------------------------------------------------------------------------
// gl_vector friends and members

gl_vector operator+(const gl_vector& v1,const gl_vector& v2)
{
	return gl_vector(v1.vec[0]+v2.vec[0], v1.vec[1]+v2.vec[1], v1.vec[2]+v2.vec[2]);
}

gl_vector operator-(const gl_vector& v1,const gl_vector& v2)
{
	return gl_vector(v1.vec[0]-v2.vec[0], v1.vec[1]-v2.vec[1], v1.vec[2]-v2.vec[2]);
}

gl_vector operator-(const gl_vector& v1)
{
	return gl_vector(-v1.vec[0],-v1.vec[1],-v1.vec[2]);
}

gl_vector operator^(const gl_vector& v1,const gl_vector& v2)
{
	return gl_vector( v1.vec[1]*v2.vec[2]-v1.vec[2]*v2.vec[1],
				  -v1.vec[0]*v2.vec[2]+v1.vec[2]*v2.vec[0],
				   v1.vec[0]*v2.vec[1]-v1.vec[1]*v2.vec[0]);
}

real operator*(const gl_vector& v1,const gl_vector& v2)
{
	return v1.vec[0]*v2.vec[0] + v1.vec[1]*v2.vec[1] + v1.vec[2]*v2.vec[2];
}

/*!
This function together with operator*(real,gl_vector) implements the commutative product of a scalar by a gl_vector
*/
gl_vector operator*(const gl_vector& v,const real& fact)
{
	return gl_vector(fact*v.vec[0],fact*v.vec[1],fact*v.vec[2]);
}

/*!
This function together with operator*(gl_vector,real) implements the commutative product of a scalar by a gl_vector
*/
gl_vector operator*(const real& fact,const gl_vector& v)
{
	return gl_vector(fact*v.vec[0],fact*v.vec[1],fact*v.vec[2]);
}

gl_vector operator/(const gl_vector& v,const real& fact)
{
	assert(fabs(fact)>=epsilon );
	return gl_vector(v.vec[0]/fact,v.vec[1]/fact,v.vec[2]/fact);
}
	gl_vector Bisect(gl_vector v0,gl_vector v1);//!< returns the unit gl_vector which halves the arc between v0 and v1

gl_vector Bisect(gl_vector v0,gl_vector v1)
{
	//add the vectors
	gl_vector v=v0+v1;
	// normalize the sum gl_vector or fill it with a standar gl_vector
	real norm=v.length2();
	if(norm<1e-5) v=Z_AXIS;
	else v/=sqrt(norm);
	return v;
}
/*
std::ostream& operator<<(std::ostream& os,const gl_vector& vect)
{
	os<<"[ "<<vect.vec[0]<<" ; "<<vect.vec[1]<<" ; "<<vect.vec[2]<<" ]";
	return os;
}

#ifdef __AFXWIN_H__ // see if we are using MFC
#ifdef _DEBUG
CDumpContext& operator<<(CDumpContext& cd,const gl_vector& vect)
{
	cd<<"[ "<<vect.vec[0]<<" ; "<<vect.vec[1]<<" ; "<<vect.vec[2]<<" ]";
	return cd;
}
#endif
#endif
*/
/*!
This function modifies the gl_vector upon which has been called.
\return the length of the gl_vector before normalization, this is useful to check if the normalization operation has been numerically precise.
*/
real gl_vector::normalize()
{
	real len=length();
	assert(fabs(len)>=epsilon);
	(*this)/=len;
	return len;
}

/*!
This function doesn't modifies the gl_vector upon which has been called, it returns a new gl_vector instead.
\return the normalized copy of the gl_vector
*/
gl_vector gl_vector::normalized() const
{
	real len=length();
	if (fabs(len)>=epsilon)
	{ 
		return gl_vector(this->vec[0]/len,this->vec[1]/len,this->vec[2]/len);
	}
	else return *this;
}

void gl_vector::EpsilonCorrect(const gl_vector& v)
{
	if(simpleabs(vec[0])<epsilon && simpleabs(vec[1])<epsilon && simpleabs(vec[2])<epsilon)	*this=v;
}


/*
operator gl_vector::operator char*()
{
}
*/

//-----------------------------------------------------------------------------
// gl_quaternion friends and members

gl_quaternion operator+(const gl_quaternion& q1,const gl_quaternion& q2)
{
	return gl_quaternion(q1.s+q2.s, q1.v+q2.v);
}

gl_quaternion operator-(const gl_quaternion& q1,const gl_quaternion& q2)
{
	return gl_quaternion(q1.s-q2.s, q1.v-q2.v);
}

gl_quaternion operator-(const gl_quaternion& q1)
{
	return gl_quaternion( -(q1.s), -(q1.v) );
}

gl_quaternion operator*(const gl_quaternion& q1,const gl_quaternion& q2)
{
	real ts=q1.s*q2.s-q1.v*q2.v;
	gl_vector tv=q1.s*q2.v+q2.s*q1.v+(q1.v^q2.v);
	return gl_quaternion(ts,tv);
}

/*!
This function together with operator*(real,gl_quaternion) implements the commutative product of a scalar by a gl_quaternion
*/
gl_quaternion operator*(const gl_quaternion& q,const real& fact)
{
	return gl_quaternion(fact*q.s,fact*q.v);
}

/*!
This function together with operator*(gl_quaternion,real) implements the commutative product of a scalar by a gl_quaternion
*/
gl_quaternion operator*(const real& fact,const gl_quaternion& q)
{
	return gl_quaternion(fact*q.s,fact*q.v);
}

gl_quaternion operator/(const gl_quaternion& q,const real& fact)
{
	return fact*q.inversed();
}

gl_quaternion operator/(const gl_quaternion& q1,const gl_quaternion& q2)
{
	return q1*q2.inversed();
}
/*
std::ostream& operator<<(std::ostream& os,const gl_quaternion& q)
{
	os<<"< "<<q.s<<" , "<<q.v<<" >";
	return os;
}

#ifdef __AFXWIN_H__ // see if we are using MFC
#ifdef _DEBUG
CDumpContext& operator<<(CDumpContext& cd,const gl_quaternion& q)
{
	cd<<"< "<<q.s<<" , "<<q.v<<" >";
	return cd;
}
#endif
#endif
*/
gl_quaternion& gl_quaternion::operator*=(const gl_quaternion& other)
{
	real temp=s;
	s=s*other.s-v*other.v;
	v=temp*other.v+other.s*v+(v^other.v);
	return *this;
}

gl_quaternion& gl_quaternion::operator/=(const gl_quaternion& other)
{
	gl_quaternion temp=other.inversed();
	real ts=s;
	s=ts*temp.s-v*temp.v;
	v=ts*temp.v+temp.s*v+(v^temp.v);
	return *this;
}
/*!
This function check if two quaternions are equal.
*/
int operator==(const gl_quaternion& q1,const gl_quaternion& q2)
{
	if(q1.s==q2.s && q1.v==q2.v) return 1;
	else return 0;
}

/*!
This function check if two quaternions are not equal.
*/
int operator!=(const gl_quaternion& q1,const gl_quaternion& q2)
{
	if(q1.s==q2.s && q1.v==q2.v) return 0;
	else return 1;
}

/*!
\b NOTE: the norm is comparable to the squared length of a gl_vector not to the length
*/
real gl_quaternion::norm() const
{
	return (s*s+v.length2());
}

/*!
This function modifies the gl_quaternion upon which has been called.
\return the length of the gl_quaternion before normalization, this is useful to check if the normalization operation has been numerically precise.
*/
real gl_quaternion::normalize()
{
	real len=length();
	assert(fabs(len)>=epsilon);
	s/=len;
	v/=len;
	return len;
}

/*!
This function doesn't modifies the gl_quaternion upon which has been called, it returns a new gl_quaternion instead.
\return the normalized copy of the gl_quaternion
*/
gl_quaternion gl_quaternion::normalized() const
{
	real len=length();
	assert(fabs(len)>=epsilon);
	return gl_quaternion(s/len,v/len);
}

/*!
This function modifies the gl_quaternion upon which has been called.
\return the norm of the gl_quaternion before the internal normalization, this is useful to check if the normalization operation has been numerically precise.
*/
real gl_quaternion::inverse()
{
	real n=norm();
	assert(fabs(n)>=epsilon);
	s/=n;
	v/=-n;
	return n;
}

/*!
This function doesn't modifies the gl_quaternion upon which has been called, it returns a new gl_quaternion instead.
*/
gl_quaternion gl_quaternion::inversed() const
{
	real n=norm();
	assert(fabs(n)>=epsilon);
	return gl_quaternion( s/n, v/(-n) );
}
/*!
Generates a rotation matrix even from non unit quaternions (only for unit
quaternoin the result is the same as gl_unitquaternion::getRotMatrix).
The generated rotation matrix is OpenGL compatible, it is intended to be
post-multiplied to the current trasformation matrix to achieve the rotation.
\return the rotation matrix
*/
gl_tmatrix gl_quaternion::getRotMatrix()
{
	gl_tmatrix result;
	real n=norm();
	real s=n>0?2.0/n:0.0;
	real xs=x()*s;	real ys=y()*s;	real zs=z()*s;
	real wx=w()*xs;	real wy=w()*ys;	real wz=w()*zs;
	real xx=x()*xs;	real xy=x()*ys;	real xz=x()*zs;
	real yy=y()*ys;	real yz=y()*zs;	real zz=z()*zs;
	result(0,0) = 1.0 - (yy + zz);
	result(1,0) = xy - wz;
	result(2,0) = xz + wy;
	result(3,0) = 0.0;

	result(0,1) = xy + wz;
	result(1,1) = 1.0 - (xx+ zz);
	result(2,1) = yz - wx;
	result(3,1) = 0.0;

	result(0,2) = xz - wy;
	result(1,2) = yz + wx;
	result(2,2) = 1.0 - (xx + yy);
	result(3,2) = 0.0;

	result(0,3) = 0.0;
	result(1,3) = 0.0;
	result(2,3) = 0.0;
	result(3,3) = 1.0;
	return result;
}


//-----------------------------------------------------------------------------
// gl_unitquaternion


/*!
The generated rotation matrix is OpenGL compatible, it is intended to be
post-multiplied to the current trasformation matrix to achieve the rotation.
\return the rotation matrix
*/
gl_tmatrix gl_unitquaternion::getRotMatrix() const
{
	gl_tmatrix result;
	register real t1,t2,t3;
/*	the code below performs the following calculations but has been
	reorganized to exploit three temporaries variables efficiently

	result(0,0) = 1.0 - 2.0*(y()*y() + z()*z());
	result(1,0) = 2.0*(x()*y() - z()*s);
	result(2,0) = 2.0*(z()*x() + y()*s);
	result(3,0) = 0.0;

	result(0,1) = 2.0*(x()*y() + z()*s);
	result(1,1) = 1.0 - 2.0*(z()*z()+ x()*x());
	result(2,1) = 2.0*(y()*z() - x()*s);
	result(3,1) = 0.0;

	result(0,2) = 2.0*(z()*x() - y()*s);
	result(1,2) = 2.0*(y()*z() + x()*s);
	result(2,2) = 1.0 - 2.0*(y()*y() + x()*x());
	result(3,2) = 0.0;

	result(0,3) = 0.0;
	result(1,3) = 0.0;
	result(2,3) = 0.0;
	result(3,3) = 1.0;
*/
	t1=2.0*x()*x();
	t2=2.0*y()*y();
	t3=2.0*z()*z();
	result(0,0) = 1.0 - t2 - t3;
	result(1,1) = 1.0 - t3 - t1;
	result(2,2) = 1.0 - t2 - t1;

	t1=2.0*x()*y();
	t2=2.0*z()*s;
	result(1,0) = t1 - t2;
	result(0,1) = t1 + t2;

	t1=2.0*z()*x();
	t2=2.0*y()*s;
	result(2,0) = t1 + t2;
	result(0,2) = t1 - t2;

	t1=2.0*y()*z();
	t2=2.0*x()*s;
	result(2,1) = t1 - t2;
	result(1,2) = t1 + t2;

	result(3,0) = 	result(3,1) = 	result(3,2) = 0.0;
	result(0,3) = 	result(1,3) = 	result(2,3) = 0.0;
	result(3,3) = 1.0;
	return result;
}


/*!
the rotation represented by the gl_unitquaternion would transform the first gl_vector
into the second. The vectors are of unit length (so they are placed on a unit sphere)
\param vfrom the first gl_vector
\param vto the second gl_vector
*/
void gl_unitquaternion::getVectorsOnSphere(gl_vector& vfrom,gl_vector& vto)
{
	gl_unitquaternion tmp=(*this)*(*this);
	real s=sqrt(tmp.x()*tmp.x()+tmp.y()*tmp.y());
	if(s<=epsilon) vfrom=Y_AXIS;
	else vfrom=gl_vector(-tmp.y()/s,tmp.x()/s,0.0);
	vto.x()=tmp.w()*vfrom.x()-tmp.z()*vfrom.y();
	vto.y()=tmp.w()*vfrom.y()+tmp.z()*vfrom.x();
	vto.z()=tmp.x()*vfrom.y()-tmp.y()*vfrom.x();
	if(w()<0.0) vfrom=-vfrom;
}
 gl_unitquaternion& gl_unitquaternion::operator*=(const real &fact)
 {
 	s*=fact;
 	v*=fact;
 	normalize();
 	return *this;
 }
gl_unitquaternion& gl_unitquaternion::operator*=(const gl_unitquaternion& other)
{
	real temp=s;
	s=s*other.s-v*other.v;
	v=temp*other.v+other.s*v+(v^other.v);
	return *this;
}

/*!
This function has been defined to trap the use of an operation which is not allowed
*/
gl_unitquaternion operator+(const gl_unitquaternion& q1,const gl_unitquaternion& q2)
{
	// THIS OPERATION IS NOT ALLOWED CAUSE DOESN'T MANTAIN THE UNIT LENGTH
	assert(false);
	return q1;
}

/*!
This function has been defined to trap the use of an operation which is not allowed
*/
gl_unitquaternion operator-(const gl_unitquaternion& q1,const gl_unitquaternion& q2)
{
	// THIS OPERATION IS NOT ALLOWED CAUSE DOESN'T MANTAIN THE UNIT LENGTH
	assert(false);
	return q1;
}


/*!
This function has been defined to trap the use of an operation which is not allowed
*/
gl_unitquaternion operator*(const gl_unitquaternion& q,const real& s)
{
	// THIS OPERATION IS NOT ALLOWED CAUSE DOESN'T MANTAIN THE UNIT LENGTH
	assert(false);
	return q;
}


/*!
This function has been defined to trap the use of an operation which is not allowed
*/
gl_unitquaternion operator*(const real& s,const gl_unitquaternion& q)
{
	// THIS OPERATION IS NOT ALLOWED CAUSE DOESN'T MANTAIN THE UNIT LENGTH
	assert(false);
	return q;
}


/*!
This function has been defined to trap the use of an operation which is not allowed
*/
gl_unitquaternion operator/(const gl_unitquaternion& q,const real& s)
{
	// THIS OPERATION IS NOT ALLOWED CAUSE DOESN'T MANTAIN THE UNIT LENGTH
	assert(false);
	return q;
}


//-----------------------------------------------------------------------------
// gl_tmatrix friends and members
/*
std::ostream& operator<<(std::ostream& os,const gl_tmatrix& m)
{
	os<<"[ "<<m.mat[0][0]<<' '<<m.mat[1][0]<<' '<<m.mat[2][0]<<' '<<m.mat[3][0]<<" ]\n";
	os<<"[ "<<m.mat[0][1]<<' '<<m.mat[1][1]<<' '<<m.mat[2][1]<<' '<<m.mat[3][1]<<" ]\n";
	os<<"[ "<<m.mat[0][2]<<' '<<m.mat[1][2]<<' '<<m.mat[2][2]<<' '<<m.mat[3][2]<<" ]\n";
	os<<"[ "<<m.mat[0][3]<<' '<<m.mat[1][3]<<' '<<m.mat[2][3]<<' '<<m.mat[3][3]<<" ]\n";
	return os;
}

#ifdef _AFXDLL // see if we are using MFC
#ifdef _DEBUG
CDumpContext& operator<<(CDumpContext& cd,const gl_tmatrix& m)
{
	cd<<"[ "<<m.mat[0][0]<<" "<<m.mat[1][0]<<" "<<m.mat[2][0]<<" "<<m.mat[3][0]<<" ]\n";
	cd<<"[ "<<m.mat[0][1]<<" "<<m.mat[1][1]<<" "<<m.mat[2][1]<<" "<<m.mat[3][1]<<" ]\n";
	cd<<"[ "<<m.mat[0][2]<<" "<<m.mat[1][2]<<" "<<m.mat[2][2]<<" "<<m.mat[3][2]<<" ]\n";
	cd<<"[ "<<m.mat[0][3]<<" "<<m.mat[1][3]<<" "<<m.mat[2][3]<<" "<<m.mat[3][3]<<" ]\n";
	return cd;
}
#endif
#endif
*/ 
void gl_tmatrix::loadIdentity()
{
	// set to zero all the elements except the diagonal
	for (int i=0;i<4;i++)
		for(int j=i+1;j<4;j++)
			mat[i][j]=mat[j][i]=0.0;
	// set to 1 the diagonal
	mat[0][0]=mat[1][1]=mat[2][2]=mat[3][3]=1.0;
}

gl_tmatrix operator+(const gl_tmatrix& t1,const gl_tmatrix& t2)
{
	gl_tmatrix temp;
	// "quick & dirty" approach accessing the internal matrices as vectors
	real* rp=(real*)temp.mat;
	real* rp1=(real*)t1.mat;
	real* rp2=(real*)t2.mat;
	for (int pos=0;pos<16;pos++)
		rp[pos]=rp1[pos]+rp2[pos];
	return temp;
}

gl_tmatrix operator-(const gl_tmatrix& t1,const gl_tmatrix& t2)
{
	gl_tmatrix temp;
	// "quick & dirty" approach accessing the internal matrices as vectors
	real* rp=(real*)temp.mat;
	real* rp1=(real*)t1.mat;
	real* rp2=(real*)t2.mat;
	for (int pos=0;pos<16;pos++)
		rp[pos]=rp1[pos]-rp2[pos];
	return temp;
}

gl_tmatrix operator*(const gl_tmatrix& t1,const gl_tmatrix& t2)
{
	gl_tmatrix temp;
	for(int c=0;c<4;c++)
		for(int r=0;r<4;r++)
		{
			temp.mat[r][c]=t1.mat[0][c]*t2.mat[r][0];
			for(int p=1;p<4;p++) temp.mat[r][c]+=t1.mat[p][c]*t2.mat[r][p];
		};
	return temp;
}

gl_tmatrix operator*(const gl_tmatrix& tmat,const real& fact)
{
	gl_tmatrix temp;
	// "quick & dirty" approach accessing the internal matrices as vectors
	real* rp=(real*)temp.mat;
	real* rp1=(real*)tmat.mat;
	for (int pos=0;pos<16;pos++)
		rp[pos]=rp1[pos]*fact;
	return temp;
}

gl_tmatrix operator*(const real& fact,const gl_tmatrix& tmat)
{
	gl_tmatrix temp;
	// "quick & dirty" approach accessing the internal matrices as vectors
	real* rp=(real*)temp.mat;
	real* rp1=(real*)tmat.mat;
	for (int pos=0;pos<16;pos++)
		rp[pos]=rp1[pos]*fact;
	return temp;
}

gl_tmatrix operator/(const gl_tmatrix& tmat,const real& fact)
{
	assert(fact>=epsilon);
	gl_tmatrix temp;
	// "quick & dirty" approach accessing the internal matrices as vectors
	real* rp=(real*)temp.mat;
	real* rp1=(real*)tmat.mat;
	for (int pos=0;pos<16;pos++)
		rp[pos]=rp1[pos]/fact;
	return temp;
}

/*!
\param bInit if set to true initializes the matrix to the identity matrix
*/
gl_tmatrix::gl_tmatrix()
{
	counter++;
}

gl_tmatrix::gl_tmatrix(const real v[16],ordermode ord)
{
	if (ord==ROW)
		for (int r=0;r<4;r++)
			for (int c=0;c<4;c++)
				mat[r][c]=v[r+4*c];
// which is faster?
/*	else
		for (int r=0;r<4;r++)
			for (int c=0;c<4;c++)
				mat[r][c]=v[c+4*r];
*/
	else
	{
		real* rp=(real*)mat;
		for (int pos=0;pos<16;pos++)
			rp[pos]=v[pos];

	}
	counter++;
}

gl_tmatrix::gl_tmatrix(const real& val)
{
	// "quick & dirty" approach accessing the internal matrix as a gl_vector
	real* rp=(real*)mat;
	for (int pos=0;pos<16;pos++)
		rp[pos]=val;
	counter++;
}

gl_tmatrix& gl_tmatrix::operator=(const gl_tmatrix& other)
{
	// "quick & dirty" approach accessing the internal matrices as vectors
	real* rp=(real*)mat;
	real* rp2=(real*)other.mat;
	for (int pos=0;pos<16;pos++)
		rp[pos]=rp2[pos];
	return *this;
}

gl_tmatrix& gl_tmatrix::operator+=(const gl_tmatrix& other)
{
	// "quick & dirty" approach accessing the internal matrices as vectors
	real* rp=(real*)mat;
	real* rp2=(real*)other.mat;
	for (int pos=0;pos<16;pos++)
		rp[pos]+=rp2[pos];
	return *this;
}

gl_tmatrix& gl_tmatrix::operator-=(const gl_tmatrix& other)
{
	// "quick & dirty" approach accessing the internal matrices as vectors
	real* rp=(real*)mat;
	real* rp2=(real*)other.mat;
	for (int pos=0;pos<16;pos++)
		rp[pos]-=rp2[pos];
	return *this;
}

gl_tmatrix& gl_tmatrix::operator*=(const gl_tmatrix& other)
{
	gl_tmatrix temp=*this;
	for(int c=0;c<4;c++)
		for(int r=0;r<4;r++)
		{
			this->mat[r][c]=temp.mat[0][c]*other.mat[r][0];
			for(int p=1;p<4;p++) this->mat[r][c]+=temp.mat[p][c]*other.mat[r][p];
		};
	return *this;
}

gl_tmatrix& gl_tmatrix::operator*=(const real& fact)
{
	// "quick & dirty" approach accessing the internal matrices as vectors
	real* rp=(real*)mat;
	for (int pos=0;pos<16;pos++)
		rp[pos]*=fact;
	return *this;
}

gl_tmatrix& gl_tmatrix::operator/=(const real& fact)
{
	assert(fact>=epsilon);
	// "quick & dirty" approach accessing the internal matrices as vectors
	real* rp=(real*)mat;
	for (int pos=0;pos<16;pos++)
		rp[pos]/=fact;
	return *this;
}

gl_tmatrix& gl_tmatrix::operator-()
{
	// "quick & dirty" approach accessing the internal matrices as vectors
	real* rp=(real*)mat;
	for (int pos=0;pos<16;pos++)
		rp[pos]=-rp[pos];
	return *this;
}

//-----------------------------------------------------------------------------
// global functions



#ifdef REAL_IS_FLOAT
// this below could be MSVC compiler specific
#pragma warning( pop )// reset the warning status
#endif
